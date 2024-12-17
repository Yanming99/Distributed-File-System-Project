#include <iostream>
#include <fstream>
#include <string>
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << argv[0] << ": Usage: diskImageFile sourceFile destinationInode" << endl;
        return 1;
    }

    // Parse command-line arguments
    string diskImage = argv[1];
    string sourcePath = argv[2];
    int destinationInode;

    try {
        destinationInode = stoi(argv[3]);
    } catch (invalid_argument &e) {
        cerr << "Could not write to dst_file" << endl;
        return 1;
    }

    // Initialize the disk and local file system
    Disk disk(diskImage, UFS_BLOCK_SIZE);
    LocalFileSystem fileSystem(&disk);

    // Open the source file
    ifstream inputFile(sourcePath, ios::binary | ios::ate);
    if (!inputFile.is_open()) {
        cerr << "Could not write to dst_file" << endl;
        return 1;
    }

    // Determine the size of the source file
    streamsize fileSize = inputFile.tellg();

    if (fileSize < 0) {
        cerr << "Could not write to dst_file" << endl;
        inputFile.close();
        return 1;
    }
    inputFile.seekg(0, ios::beg);

    // Allocate buffer and read the source file
    char *buffer = nullptr;
    if (fileSize > 0) {
        try {
            buffer = new char[fileSize];
        } catch (bad_alloc &e) {
            cerr << "Could not write to dst_file" << endl;
            inputFile.close();
            return 1;
        }

        if (!inputFile.read(buffer, fileSize)) {
            cerr << "Could not write to dst_file" << endl;
            delete[] buffer;
            inputFile.close();
            return 1;
        }
    }
    inputFile.close();

    // Write the file contents to the destination inode
    int writtenBytes = fileSystem.write(destinationInode, buffer, fileSize);
    if(writtenBytes <-1) {
        cerr << "Could not write to dst_file" << endl;
        return 1;
    }


    if (buffer != nullptr) {
        delete[] buffer;
    }

    return 0;
}
