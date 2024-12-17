#include <iostream>
#include <string>
#include <cstring>
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[]) {
    // Validate arguments
    if (argc != 3) {
        cerr << "Error reading file" << endl;
        return 1;
    }

    int inodeNumber;
    try {
        inodeNumber = stoi(argv[2]);
    } catch (...) {
        cerr << "Error reading file" << endl;
        return 1;
    }

    // Initialize disk and file system
    Disk disk(argv[1], UFS_BLOCK_SIZE);
    LocalFileSystem fileSystem(&disk);

    // Read superblock to validate block numbers later
    super_t super;
    fileSystem.readSuperBlock(&super);

    // Retrieve inode metadata
    inode_t inode;
    if (fileSystem.stat(inodeNumber, &inode) < 0) {
        cerr << "Error reading file" << endl;
        return 1;
    }

    // Error if the inode represents a directory
    if (inode.type == UFS_DIRECTORY) {
        cerr << "Error reading file" << endl;
        return 1;
    }

    // Print file blocks
    cout << "File blocks" << endl;
    int blocks = inode.size / UFS_BLOCK_SIZE;
    if ((inodeNumber % UFS_BLOCK_SIZE) != 0) {
        blocks += 1;
    }
    for (int i = 0; i < blocks ; i++) {
        if (inode.direct[i] >= static_cast<unsigned int>(super.data_region_addr) &&
            inode.direct[i] < static_cast<unsigned int>(super.data_region_addr + super.data_region_len)) {
            cout << inode.direct[i] << endl;
        }
    }
    cout << endl; // Blank line after blocks

    // Print file data
    cout << "File data" << endl;
    char buffer[inode.size]; // Allocate buffer to hold the entire file
    int bytesRead = fileSystem.read(inodeNumber, buffer, inode.size);
    if (bytesRead < 0) {
        cerr << "Error reading file" << endl;
        return 1;
    }

    // Output file data to standard output
    cout.write(buffer, bytesRead);

    return 0;
}
