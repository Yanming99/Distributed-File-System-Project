#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

void printBitmap(unsigned char *bitmap, int totalItems) {
    int totalBytes = (totalItems + 7) / 8;
    for (int i = 0; i < totalBytes; i++) {
        cout << static_cast<unsigned int>(bitmap[i]) << " ";
    }
    cout << endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1; 
    }

    // Initialize disk and file system
    Disk disk(argv[1], UFS_BLOCK_SIZE);
    LocalFileSystem fileSystem(&disk);

    // Read the superblock
    super_t super;
    fileSystem.readSuperBlock(&super);

    // Print superblock fields
    cout << "Super" << endl;
    cout << "inode_region_addr " << super.inode_region_addr << endl;
    cout << "inode_region_len " << super.inode_region_len << endl;
    cout << "num_inodes " << super.num_inodes << endl;
    cout << "data_region_addr " << super.data_region_addr << endl;
    cout << "data_region_len " << super.data_region_len << endl;
    cout << "num_data " << super.num_data << endl;

    // Print inode bitmap
    unsigned char *inodeBitmap = new unsigned char[super.inode_bitmap_len * UFS_BLOCK_SIZE]();
    fileSystem.readInodeBitmap(&super, inodeBitmap);
    cout << endl << "Inode bitmap" << endl;
    printBitmap(inodeBitmap, super.num_inodes);

    // Print data bitmap
    unsigned char *dataBitmap = new unsigned char[super.data_bitmap_len * UFS_BLOCK_SIZE]();
    fileSystem.readDataBitmap(&super, dataBitmap);
    cout << endl << "Data bitmap" << endl;
    printBitmap(dataBitmap, super.num_data);

    // Cleanup
    delete[] inodeBitmap;
    delete[] dataBitmap;

    return 0;
}
