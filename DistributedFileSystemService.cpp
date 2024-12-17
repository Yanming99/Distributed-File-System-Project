#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <algorithm>

#include "DistributedFileSystemService.h"
#include "ClientError.h"
#include "ufs.h"
#include "WwwFormEncodedDict.h"

using namespace std;

DistributedFileSystemService::DistributedFileSystemService(string diskFile) : HttpService("/ds3/") {
    this->fileSystem = new LocalFileSystem(new Disk(diskFile, UFS_BLOCK_SIZE));
}

void DistributedFileSystemService::get(HTTPRequest *request, HTTPResponse *response) {
    string path = request->getPath().substr(this->pathPrefix().size());
    vector<string> components = StringUtils::split(path, '/');
    int currentInode = UFS_ROOT_DIRECTORY_INODE_NUMBER;

    try {
        for (const string &component : components) {
            if (!component.empty()) {
                currentInode = fileSystem->lookup(currentInode, component);
            }
        }

        inode_t inode;
        if (fileSystem->stat(currentInode, &inode) < 0) {
            throw ClientError::notFound();
        }

        if (inode.type == UFS_REGULAR_FILE) {
            char buffer[inode.size];
            int bytesRead = fileSystem->read(currentInode, buffer, inode.size);
            if (bytesRead < 0) throw ClientError::notFound();
            response->setBody(string(buffer, bytesRead));
        } else if (inode.type == UFS_DIRECTORY) {
            unsigned char dirBuffer[inode.size];
            int bytesRead = fileSystem->read(currentInode, dirBuffer, inode.size);
            if (bytesRead < 0) throw ClientError::notFound();

            dir_ent_t *entries = reinterpret_cast<dir_ent_t *>(dirBuffer);
            int numEntries = bytesRead / sizeof(dir_ent_t);

            stringstream body;
            for (int i = 0; i < numEntries; i++) {
                if (entries[i].inum != 0) {
                    body << entries[i].name;
                    if (fileSystem->stat(entries[i].inum, &inode) == 0 && inode.type == UFS_DIRECTORY) {
                        body << "/";
                    }
                    body << "\n";
                }
            }

            response->setBody(body.str());
        }
    } catch (ClientError &e) {
        throw e;  // Re-throw for framework handling
    }
}

void DistributedFileSystemService::put(HTTPRequest *request, HTTPResponse *response) {
    string path = request->getPath().substr(this->pathPrefix().size());
    vector<string> components = StringUtils::split(path, '/');
    int currentInode = UFS_ROOT_DIRECTORY_INODE_NUMBER;
    string fileName = components.back();
    components.pop_back();

    // Comment out or implement transaction handling
    // this->fileSystem->disk->beginTransaction();
    try {
        for (const string &component : components) {
            if (!component.empty()) {
                int nextInode = fileSystem->lookup(currentInode, component);
                if (nextInode < 0) {  // Create directory if it doesn't exist
                    currentInode = fileSystem->create(currentInode, UFS_DIRECTORY, component);
                } else {
                    inode_t inode;
                    fileSystem->stat(nextInode, &inode);
                    if (inode.type != UFS_DIRECTORY) {
                        throw ClientError::conflict();
                    }
                    currentInode = nextInode;
                }
            }
        }

        int fileInode = fileSystem->lookup(currentInode, fileName);
        if (fileInode < 0) {  // File does not exist
            fileInode = fileSystem->create(currentInode, UFS_REGULAR_FILE, fileName);
        }

        string body = request->getBody();
        fileSystem->write(fileInode, body.c_str(), body.size());
        // this->fileSystem->disk->commitTransaction();
    } catch (ClientError &e) {
        // this->fileSystem->disk->rollbackTransaction();
        throw e;
    }
}

void DistributedFileSystemService::del(HTTPRequest *request, HTTPResponse *response) {
    string path = request->getPath().substr(this->pathPrefix().size());
    vector<string> components = StringUtils::split(path, '/');
    int currentInode = UFS_ROOT_DIRECTORY_INODE_NUMBER;
    string targetName = components.back();
    components.pop_back();

    // Comment out or implement transaction handling
    // this->fileSystem->disk->beginTransaction();
    try {
        for (const string &component : components) {
            if (!component.empty()) {
                currentInode = fileSystem->lookup(currentInode, component);
                if (currentInode < 0) throw ClientError::notFound();
            }
        }

        int targetInode = fileSystem->lookup(currentInode, targetName);
        if (targetInode < 0) throw ClientError::notFound();

        inode_t inode;
        fileSystem->stat(targetInode, &inode);
        if (inode.type == UFS_DIRECTORY && inode.size > 0) {
            throw ClientError::conflict();
        }

        fileSystem->unlink(currentInode, targetName);
        // this->fileSystem->disk->commitTransaction();
    } catch (ClientError &e) {
        // this->fileSystem->disk->rollbackTransaction();
        throw e;
    }
}