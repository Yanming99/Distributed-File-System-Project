#include <iostream>
#include <string>
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "Error creating directory" << endl;
        return 1;
    }

    string diskImageFile = argv[1];
    int parentInode;
    string directory = argv[3];

    try {
        parentInode = stoi(argv[2]);
    } catch (...) {
        cerr << "Error creating directory" << endl;
        return 1;
    }

    Disk *disk = nullptr;
    LocalFileSystem *fileSystem = nullptr;

    try {
        disk = new Disk(diskImageFile, UFS_BLOCK_SIZE);
        fileSystem = new LocalFileSystem(disk);
    } catch (...) {
        cerr << "Error creating directory" << endl;
        delete disk;
        return 1;
    }

    int result = fileSystem->create(parentInode, UFS_DIRECTORY, directory);

    if (result < 0) {
        cerr << "Error creating directory" << endl;
        delete fileSystem;
        delete disk;
        return 1;
    }

    delete fileSystem;
    delete disk;

    return 0;
}
