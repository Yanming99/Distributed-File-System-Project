#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "StringUtils.h"
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

// Use this function with std::sort for directory entries
bool compareByName(const dir_ent_t &a, const dir_ent_t &b) {
    return std::strcmp(a.name, b.name) < 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << argv[0] << ": diskImageFile directory" << endl;
        cerr << "For example:" << endl;
        cerr << "    $ " << argv[0] << " tests/disk_images/a.img /a/b" << endl;
        return 1;
    }

    // Initialize the disk and file system
    Disk disk(argv[1], UFS_BLOCK_SIZE);
    LocalFileSystem fileSystem(&disk);
    string path = argv[2];

    // Parse the path
    vector<string> components = StringUtils::split(path, '/');
    int currentInode = UFS_ROOT_DIRECTORY_INODE_NUMBER;

    // Traverse the path
for (const string &component : components) {
    if (component.empty()) continue; // Skip empty components for root or multiple slashes

    int nextInode = fileSystem.lookup(currentInode, component);
    if (nextInode < 0) { // Lookup failed
        cerr << "Directory not found" << endl;
        return 1;
    }
    currentInode = nextInode;
}


    // Retrieve inode metadata
    inode_t inode;
    if (fileSystem.stat(currentInode, &inode) < 0) {
        cerr << "Directory not found" << endl;
        return 1;
    }

if (inode.type == UFS_DIRECTORY) {
    unsigned char buffer[inode.size];
    int bytesRead = fileSystem.read(currentInode, buffer, inode.size);

    if (bytesRead < 0) {
        cerr << "Directory not found" << endl;
        return 1;
    }

    dir_ent_t *entries = reinterpret_cast<dir_ent_t *>(buffer);
    int numEntries = bytesRead / sizeof(dir_ent_t);

    vector<dir_ent_t> entryList(entries, entries + numEntries);
    std::sort(entryList.begin(), entryList.end(), compareByName);

    for (const dir_ent_t &entry : entryList) {
        if ( entry.name[0] != '\0') { // Allow 0 as it is valid for `.` and `..`
            cout << entry.inum << "\t" << entry.name << endl;
        }
    }
} else {
    // Extract only the last component of the path to print the file name
    size_t lastSlash = path.find_last_of('/');
    string fileName = (lastSlash == string::npos) ? path : path.substr(lastSlash + 1);

    // Print the inode number and the file name
    cout << currentInode << "\t" << fileName << endl;
}


    return 0;
}