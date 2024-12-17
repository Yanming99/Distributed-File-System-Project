#include <iostream>
#include <string>
#include <cstdlib>
#include "Disk.h"
#include "LocalFileSystem.h"
#include "ufs.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Error removing entry" << std::endl;
        return 1;
    }

    std::string diskImageFileName = argv[1];
    int parentInodeNumber = atoi(argv[2]);
    std::string name = argv[3];

    int blockSize = UFS_BLOCK_SIZE; // Assuming block size is defined in ufs.h

    Disk *disk = nullptr;
    try {
        disk = new Disk(diskImageFileName, blockSize);
    } catch (...) {
        std::cerr << "Error removing entry" << std::endl;
        if (disk) delete disk;
        return 1;
    }

    LocalFileSystem *fs = new LocalFileSystem(disk);
    int ret = fs->unlink(parentInodeNumber, name);
    if (ret != 0) {
        std::cerr << "Error removing entry" << std::endl;
        delete fs;
        delete disk;
        return 0;
    }

    delete fs;
    delete disk;
    return 0;
}