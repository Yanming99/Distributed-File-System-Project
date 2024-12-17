#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <cstring>
#include <climits>
#include <algorithm>

#include "LocalFileSystem.h"
#include "ufs.h"


using namespace std;


LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}




void LocalFileSystem::readSuperBlock(super_t *super) {
    unsigned char buffer[UFS_BLOCK_SIZE];
    disk->readBlock(0, buffer);
    memcpy(super, buffer, sizeof(super_t));
}

void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
    int bitmapSize = (super->num_inodes + 7) / 8;  // Size of bitmap in bytes
    unsigned char *tempBitmap = new unsigned char[super->inode_bitmap_len * UFS_BLOCK_SIZE]();

    for (int i = 0; i < super->inode_bitmap_len; i++) {
        disk->readBlock(super->inode_bitmap_addr + i, tempBitmap + i * UFS_BLOCK_SIZE);
    }

    // Copy only the relevant bytes of the bitmap
    memcpy(inodeBitmap, tempBitmap, bitmapSize);

    delete[] tempBitmap;  // Free the temporary memory
}


void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
    int bitmapSize = (super->num_inodes + 7) / 8;  // Size of bitmap in bytes
    unsigned char *tempBitmap = new unsigned char[super->inode_bitmap_len * UFS_BLOCK_SIZE]();
    
    // Read the full bitmap into tempBitmap
    for (int i = 0; i < super->inode_bitmap_len; i++) {
        disk->readBlock(super->inode_bitmap_addr + i, tempBitmap + i * UFS_BLOCK_SIZE);
    }

    // Copy the updated bitmap into the correct portion
    memcpy(tempBitmap, inodeBitmap, bitmapSize);

    // Write back the modified bitmap
    for (int i = 0; i < super->inode_bitmap_len; i++) {
        disk->writeBlock(super->inode_bitmap_addr + i, tempBitmap + i * UFS_BLOCK_SIZE);
    }

    delete[] tempBitmap;  // Free the temporary memory
}


void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap) {
    int bitmapSize = (super->num_data + 7) / 8;  // Size of bitmap in bytes
    unsigned char *tempBitmap = new unsigned char[super->data_bitmap_len * UFS_BLOCK_SIZE]();

    for (int i = 0; i < super->data_bitmap_len; i++) {
        disk->readBlock(super->data_bitmap_addr + i, tempBitmap + i * UFS_BLOCK_SIZE);
    }

    // Copy only the relevant bytes of the bitmap
    memcpy(dataBitmap, tempBitmap, bitmapSize);

    delete[] tempBitmap;  // Free the temporary memory
}



void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap) {
    int bitmapSize = (super->num_data + 7) / 8;  // Size of bitmap in bytes
    unsigned char *tempBitmap = new unsigned char[super->data_bitmap_len * UFS_BLOCK_SIZE]();

    // Read the full bitmap into tempBitmap
    for (int i = 0; i < super->data_bitmap_len; i++) {
        disk->readBlock(super->data_bitmap_addr + i, tempBitmap + i * UFS_BLOCK_SIZE);
    }

    // Copy the updated bitmap into the correct portion
    memcpy(tempBitmap, dataBitmap, bitmapSize);

    // Write back the modified bitmap
    for (int i = 0; i < super->data_bitmap_len; i++) {
        disk->writeBlock(super->data_bitmap_addr + i, tempBitmap + i * UFS_BLOCK_SIZE);
    }

    delete[] tempBitmap;  // Free the temporary memory
}


void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes) {
    int regionSize = super->num_inodes * sizeof(inode_t);  // Total size in bytes
    unsigned char *tempRegion = new unsigned char[super->inode_region_len * UFS_BLOCK_SIZE]();

    for (int i = 0; i < super->inode_region_len; i++) {
        disk->readBlock(super->inode_region_addr + i, tempRegion + i * UFS_BLOCK_SIZE);
    }

    // Copy only the relevant portion of the inode region
    memcpy(inodes, tempRegion, regionSize);

    delete[] tempRegion;  // Free the temporary memory
}


void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes) {
    int regionSize = super->num_inodes * sizeof(inode_t);  // Total size in bytes
    unsigned char *tempRegion = new unsigned char[super->inode_region_len * UFS_BLOCK_SIZE]();

    // Read the full inode region into tempRegion
    for (int i = 0; i < super->inode_region_len; i++) {
        disk->readBlock(super->inode_region_addr + i, tempRegion + i * UFS_BLOCK_SIZE);
    }

    // Copy the updated inodes into the correct portion
    memcpy(tempRegion, inodes, regionSize);

    // Write back the modified inode region
    for (int i = 0; i < super->inode_region_len; i++) {
        disk->writeBlock(super->inode_region_addr + i, tempRegion + i * UFS_BLOCK_SIZE);
    }

    delete[] tempRegion;  // Free the temporary memory
}





int LocalFileSystem::lookup(int parentInodeNumber, std::string name) {
    super_t super;
    readSuperBlock(&super);

    // Validate the parent inode number
    if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes) {
        return -EINVALIDINODE;
    }

    inode_t parentInode;
    if (stat(parentInodeNumber, &parentInode) < 0) {
        return -EINVALIDINODE;
    }

    // Ensure the parent inode is a directory
    if (parentInode.type != UFS_DIRECTORY) {
        return -EINVALIDINODE;
    }


    int numBlocks = parentInode.size / UFS_BLOCK_SIZE;
    if (parentInode.size % UFS_BLOCK_SIZE != 0) {
        numBlocks += 1;
    }
    for (int i = 0; i < numBlocks; i++) {  // Iterate only over valid blocks
        if (parentInode.direct[i] == 0 || parentInode.direct[i] == UINT_MAX) {
            continue;  // Skip unused or invalid block pointers
        }

        unsigned char block[UFS_BLOCK_SIZE];
        disk->readBlock(parentInode.direct[i], block);

        dir_ent_t *entries = reinterpret_cast<dir_ent_t *>(block);
        int numEntries = UFS_BLOCK_SIZE / sizeof(dir_ent_t);

        for (int j = 0; j < numEntries; j++) {
            if (entries[j].inum != 0 && name == std::string(entries[j].name)) {
                return entries[j].inum; // Found the entry
            }
        }
    }

    return -ENOTFOUND;
}











int LocalFileSystem::stat(int inodeNumber, inode_t *inode) {
    super_t super;
    readSuperBlock(&super);

    // Validate the inode number
    if (inodeNumber < 0 || inodeNumber >= super.num_inodes) {
        return -EINVALIDINODE;
    }

    // Calculate the block and offset within the inode region
    int blockIndex = (inodeNumber * sizeof(inode_t)) / UFS_BLOCK_SIZE;
    int blockOffset = (inodeNumber * sizeof(inode_t)) % UFS_BLOCK_SIZE;

    // Translate block-based address to absolute block number
    int absoluteBlockNumber = super.inode_region_addr + blockIndex;

    // Validate that the block index is within bounds
    if (absoluteBlockNumber >= super.inode_region_addr + super.inode_region_len) {
        return -EINVALIDINODE;
    }

    // Read the block containing the inode
    unsigned char buffer[UFS_BLOCK_SIZE];
    disk->readBlock(absoluteBlockNumber, buffer);

    // Copy the inode's data from the block
    memcpy(inode, buffer + blockOffset, sizeof(inode_t));   

    return 0;
}







int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
    inode_t inode;

    // Fetch the inode metadata
    if (stat(inodeNumber, &inode) < 0) {
        return -EINVALIDINODE;
    }

    // Validate the read size
    if (size < 0 || size > inode.size) {
        return -EINVALIDSIZE;
    }

    char *buf = static_cast<char *>(buffer);
    int bytesRead = 0;

    // Iterate over direct block pointers
    for (int i = 0; i < DIRECT_PTRS && bytesRead < size; i++) {
        if (inode.direct[i] == 0 || inode.direct[i] == UINT_MAX ||
            inode.direct[i] >= static_cast<unsigned int>(disk->numberOfBlocks())) {
            continue;  // Skip invalid or unused block pointers
        }

        char blockBuffer[UFS_BLOCK_SIZE];
        disk->readBlock(inode.direct[i], blockBuffer);

        // Determine the number of bytes to read from the block
        int toRead = std::min(size - bytesRead, UFS_BLOCK_SIZE);
        memcpy(buf + bytesRead, blockBuffer, toRead);
        bytesRead += toRead;
    }

    return bytesRead;
}










int LocalFileSystem::create(int parentInodeNumber, int type, std::string name) {
    // Validate the name length
    if (name.empty() || name.length() >= DIR_ENT_NAME_SIZE) {
        return -EINVALIDNAME;
    }

    // Load the superblock
    super_t super;
    readSuperBlock(&super);

    // Allocate memory for bitmaps and inodes
    int inodeBitmapSize = (super.num_inodes + 7) / 8;
    unsigned char *inodeBitmap = new unsigned char[inodeBitmapSize];
    readInodeBitmap(&super, inodeBitmap);

    int dataBitmapSize = (super.num_data + 7) / 8;
    unsigned char *dataBitmap = new unsigned char[dataBitmapSize];
    readDataBitmap(&super, dataBitmap);

    inode_t *inodes = new inode_t[super.num_inodes];
    readInodeRegion(&super, inodes);

    // Validate parentInodeNumber and get parentInode
    if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes ||
        !(inodeBitmap[parentInodeNumber / 8] & (1 << (parentInodeNumber % 8)))) {
        // Clean up
        delete[] inodeBitmap;
        delete[] dataBitmap;
        delete[] inodes;
        return -EINVALIDINODE;
    }

    inode_t &parentInode = inodes[parentInodeNumber];

    if (parentInode.type != UFS_DIRECTORY) {
        // Clean up
        delete[] inodeBitmap;
        delete[] dataBitmap;
        delete[] inodes;
        return -EINVALIDINODE;
    }

    // Check if the name already exists in the parent directory
    char buffer[UFS_BLOCK_SIZE];
    int numEntriesPerBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t);

    int numBlocks = (parentInode.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;


    for (int i = 0; i < numBlocks; i++) {
        if (parentInode.direct[i] == 0) {
            continue;
        }

        disk->readBlock(parentInode.direct[i], buffer);
        dir_ent_t *entries = (dir_ent_t *)buffer;

        for (int j = 0; j < numEntriesPerBlock; j++) {
            if (entries[j].inum != -1 && name == entries[j].name) {
                int existingInodeNumber = entries[j].inum;

                if (existingInodeNumber < 0 || existingInodeNumber >= super.num_inodes ||
                    !(inodeBitmap[existingInodeNumber / 8] & (1 << (existingInodeNumber % 8)))) {
                    // Clean up
                    delete[] inodeBitmap;
                    delete[] dataBitmap;
                    delete[] inodes;
                    return -EINVALIDINODE;
                }

                inode_t &existingInode = inodes[existingInodeNumber];

                if (existingInode.type == type) {
                    // Clean up
                    delete[] inodeBitmap;
                    delete[] dataBitmap;
                    delete[] inodes;
                    return existingInodeNumber;
                } else {
                    // Clean up
                    delete[] inodeBitmap;
                    delete[] dataBitmap;
                    delete[] inodes;
                    return -EINVALIDTYPE;
                }
            }
        }
    }

    // Allocate a new inode
    int newInodeIndex = -1;

    for (int i = 0; i < super.num_inodes; i++) {
        if (!(inodeBitmap[i / 8] & (1 << (i % 8)))) {
            newInodeIndex = i;
            inodeBitmap[i / 8] |= (1 << (i % 8));
            break;
        }
    }
    if (newInodeIndex == -1) {
        // Clean up
        delete[] inodeBitmap;
        delete[] dataBitmap;
        delete[] inodes;
        return -ENOTENOUGHSPACE;
    }

    // Initialize the new inode
    inode_t &newInode = inodes[newInodeIndex];
    memset(&newInode, 0, sizeof(inode_t));
    newInode.type = type;
    newInode.size = 0;

    if (type == UFS_DIRECTORY) {
        // Allocate a data block for the new directory
        int newBlockIndex = -1;
        for (int i = 0; i < super.num_data; i++) {
            if (!(dataBitmap[i / 8] & (1 << (i % 8)))) {
                newBlockIndex = i;
                dataBitmap[i / 8] |= (1 << (i % 8));
                break;
            }
        }
        if (newBlockIndex == -1) {
            // Clean up
            delete[] inodeBitmap;
            delete[] dataBitmap;
            delete[] inodes;
            return -ENOTENOUGHSPACE;
        }

        dir_ent_t newDirEntries[numEntriesPerBlock];
        for (int k = 0; k < numEntriesPerBlock; k++) {
            newDirEntries[k].inum = -1; // Mark as unused
            memset(newDirEntries[k].name, 0, DIR_ENT_NAME_SIZE);
        }
        strncpy(newDirEntries[0].name, ".", DIR_ENT_NAME_SIZE - 1);
        newDirEntries[0].name[DIR_ENT_NAME_SIZE - 1] = '\0';
        newDirEntries[0].inum = newInodeIndex;

        strncpy(newDirEntries[1].name, "..", DIR_ENT_NAME_SIZE - 1);
        newDirEntries[1].name[DIR_ENT_NAME_SIZE - 1] = '\0';
        newDirEntries[1].inum = parentInodeNumber;

        int newBlockNum = super.data_region_addr + newBlockIndex;
        disk->writeBlock(newBlockNum, newDirEntries);

        newInode.direct[0] = newBlockNum;
        newInode.size = 2 * sizeof(dir_ent_t);
    }

    // Try to add the new entry to the parent directory
    bool added = false;
    for (int i = 0; i < numBlocks; i++) {
        if (parentInode.direct[i] == 0) {
            continue;
        }

        disk->readBlock(parentInode.direct[i], buffer);
        dir_ent_t *entries = (dir_ent_t *)buffer;

        for (int j = 0; j < numEntriesPerBlock; j++) {
            if (entries[j].inum == -1) {
                strncpy(entries[j].name, name.c_str(), DIR_ENT_NAME_SIZE - 1);
                entries[j].name[DIR_ENT_NAME_SIZE - 1] = '\0';
                entries[j].inum = newInodeIndex;
                disk->writeBlock(parentInode.direct[i], buffer);
                parentInode.size += sizeof(dir_ent_t);
                added = true;
                break;
            }
        }
        if (added) {
            break;
        }
    }

    if (!added) {
        // Need to allocate a new data block for the parent directory
        if (numBlocks >= DIRECT_PTRS) {
            // Cannot allocate more blocks
            // Clean up
            delete[] inodeBitmap;
            delete[] dataBitmap;
            delete[] inodes;
            return -ENOTENOUGHSPACE;
        }

        // Allocate new data block
        int newBlockIndex = -1;
        for (int i = 0; i < super.num_data; i++) {
            if (!(dataBitmap[i / 8] & (1 << (i % 8)))) {
                newBlockIndex = i;
                dataBitmap[i / 8] |= (1 << (i % 8));
                break;
            }
        }
        if (newBlockIndex == -1) {
            // Clean up
            delete[] inodeBitmap;
            delete[] dataBitmap;
            delete[] inodes;
            return -ENOTENOUGHSPACE;
        }

        int newBlockNum = super.data_region_addr + newBlockIndex;
        parentInode.direct[numBlocks] = newBlockNum; // Use numBlocks as the index
        memset(buffer, 0, UFS_BLOCK_SIZE);

        dir_ent_t *entries = (dir_ent_t *)buffer;
        for (int k = 0; k < numEntriesPerBlock; k++) {
            entries[k].inum = -1; // Initialize entries as unused
            memset(entries[k].name, 0, DIR_ENT_NAME_SIZE);
        }
        strncpy(entries[0].name, name.c_str(), DIR_ENT_NAME_SIZE - 1);
        entries[0].name[DIR_ENT_NAME_SIZE - 1] = '\0';
        entries[0].inum = newInodeIndex;

        disk->writeBlock(newBlockNum, buffer);
        parentInode.size += sizeof(dir_ent_t);

    }


       inodes[parentInodeNumber] =  parentInode;
        inodes[newInodeIndex] = newInode;
    // Write back changes
    writeInodeBitmap(&super, inodeBitmap);
    writeDataBitmap(&super, dataBitmap);
    writeInodeRegion(&super, inodes);

    // Clean up
    delete[] inodeBitmap;
    delete[] dataBitmap;
    delete[] inodes;

    return newInodeIndex;
}














int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
    // Check for invalid size
    if (size < 0) {
        return -EINVALIDSIZE;
    }

    // Get the inode
    inode_t inode;
    int ret = stat(inodeNumber, &inode);
    if (ret < 0) {
        return -EINVALIDINODE;
    }

    // Check inode type
    if (inode.type != UFS_REGULAR_FILE) {
        return -EINVALIDTYPE;
    }

    // Read the superblock
    super_t super;
    readSuperBlock(&super);

    // Calculate blocks needed
    int blocksNeeded = (size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
    int currentBlocks = (inode.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;

    // Check if the file size exceeds maximum allowed size
    if (blocksNeeded > DIRECT_PTRS) {
        return -EINVALIDSIZE; // Or define a specific error code for exceeding max file size
    }

    // Read data bitmap
    int dataBitmapSize = (super.num_data + 7) / 8;
    unsigned char *dataBitmap = new unsigned char[dataBitmapSize];
    readDataBitmap(&super, dataBitmap);

    const char *data = static_cast<const char *>(buffer);

    // Allocate or reuse data blocks
    for (int i = 0; i < blocksNeeded; i++) {
        int blockOffset = i * UFS_BLOCK_SIZE;
        int bytesToWrite = std::min(UFS_BLOCK_SIZE, size - blockOffset);

        if (i < currentBlocks && inode.direct[i] != 0) {
            // Reuse existing block
            if (inode.direct[i] < static_cast<unsigned int>(super.data_region_addr) ||
                inode.direct[i] >= static_cast<unsigned int>(super.data_region_addr + super.data_region_len)) {
                delete[] dataBitmap;
                return -EINVALIDINODE;
            }

            // Write data to existing block
            if (bytesToWrite < UFS_BLOCK_SIZE) {
                char tempBuffer[UFS_BLOCK_SIZE] = {0};
                std::memcpy(tempBuffer, data + blockOffset, bytesToWrite);
                disk->writeBlock(inode.direct[i], tempBuffer);
            } else {
                disk->writeBlock(inode.direct[i], (void *)(data + blockOffset));
            }
        } else {
            // Allocate a new data block
            int newBlockIndex = -1;
            for (int j = 0; j < super.num_data; j++) {
                if (!(dataBitmap[j / 8] & (1 << (j % 8)))) {
                    dataBitmap[j / 8] |= (1 << (j % 8));
                    newBlockIndex = j;
                    break;
                }
            }
            if (newBlockIndex == -1) {
                delete[] dataBitmap;
                return -ENOTENOUGHSPACE;
            }

            int newBlockNum = super.data_region_addr + newBlockIndex;
            inode.direct[i] = newBlockNum;

            // Write data to new block
            if (bytesToWrite < UFS_BLOCK_SIZE) {
                char tempBuffer[UFS_BLOCK_SIZE] = {0};
                std::memcpy(tempBuffer, data + blockOffset, bytesToWrite);
                disk->writeBlock(newBlockNum, tempBuffer);
            } else {
                disk->writeBlock(newBlockNum, (void *)(data + blockOffset));
            }
        }
    }

    // Free unused data blocks if the new size is smaller
    for (int i = blocksNeeded; i < currentBlocks; i++) {
        if (inode.direct[i] != 0) {
            // Mark the data block as free in the data bitmap
            int dataBlockIndex = inode.direct[i] - super.data_region_addr;
            dataBitmap[dataBlockIndex / 8] &= ~(1 << (dataBlockIndex % 8));
            // Clear the direct pointer
            inode.direct[i] = 0;
        }
    }

    // Update inode size
    inode.size = size;


    // Write back the data bitmap
    writeDataBitmap(&super, dataBitmap);
    delete[] dataBitmap;

    // Read and update the inode region
    inode_t *inodes = new inode_t[super.num_inodes];
    readInodeRegion(&super, inodes);
    inodes[inodeNumber] = inode;
    writeInodeRegion(&super, inodes);
    delete[] inodes;

    if(size<0) return -EINVALIDSIZE;

    return size;
}


























int LocalFileSystem::unlink(int parentInodeNumber, std::string name) {
    // Step 1: Read superblock
    super_t super;
    readSuperBlock(&super);

    // Step 2: Read inode bitmap
    int inodeBitmapSize = (super.num_inodes + 7) / 8;
    unsigned char *inodeBitmap = new unsigned char[inodeBitmapSize];
    readInodeBitmap(&super, inodeBitmap);

    // Step 3: Read inodes
    inode_t *inodes = new inode_t[super.num_inodes];
    readInodeRegion(&super, inodes);

    // Step 4: Validate parentInodeNumber
    if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes) {
        delete[] inodeBitmap;
        delete[] inodes;
        return -EINVALIDINODE;
    }

    // Check if parent inode is allocated
    if (!(inodeBitmap[parentInodeNumber / 8] & (1 << (parentInodeNumber % 8)))) {
        delete[] inodeBitmap;
        delete[] inodes;
        return -ENOTALLOCATED;
    }

    // Check if parentInode is a directory
    if (inodes[parentInodeNumber].type != UFS_DIRECTORY) {
        delete[] inodeBitmap;
        delete[] inodes;
        return -EINVALIDINODE;
    }

    // Step 5: Check if name is valid and not "." or ".."
    if (name == "." || name == ".." || name.empty() || name.length() >= DIR_ENT_NAME_SIZE) {
        delete[] inodeBitmap;
        delete[] inodes;
        return -EUNLINKNOTALLOWED;
    }

    // Step 6: Read the directory entries of parentInode
    int dirSize = inodes[parentInodeNumber].size;
    int totalBlocks = (dirSize + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
    int maxEntriesPerBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t);

    char *dirData = new char[dirSize];
    int bytesRead = 0;

    for (int i = 0; i < totalBlocks && bytesRead < dirSize; ++i) {
        unsigned int blockNum = inodes[parentInodeNumber].direct[i];
        if (blockNum == 0) continue;

        char blockData[UFS_BLOCK_SIZE];
        disk->readBlock(blockNum, blockData);
        int toRead = std::min(UFS_BLOCK_SIZE, dirSize - bytesRead);
        memcpy(dirData + bytesRead, blockData, toRead);
        bytesRead += toRead;
    }

    // Interpret dirData as an array of dir_ent_t
    dir_ent_t *dirEntries = (dir_ent_t *)dirData;
    int numEntries = dirSize / sizeof(dir_ent_t);

    // Find the entry to unlink
    int entryIndex = -1;
    for (int i = 0; i < numEntries; ++i) {
        if (std::string(dirEntries[i].name) == name) {
            entryIndex = i;
            break;
        }
    }

    // If entry not found, return success (not an error per spec)
    if (entryIndex == -1) {
        delete[] dirData;
        delete[] inodeBitmap;
        delete[] inodes;
        return 0;
    }

    int entryInodeNumber = dirEntries[entryIndex].inum;

    // Step 8: Validate entryInodeNumber
    if (entryInodeNumber < 0 || entryInodeNumber >= super.num_inodes) {
        delete[] dirData;
        delete[] inodeBitmap;
        delete[] inodes;
        return -EINVALIDINODE;
    }

    // Check if entry inode is allocated
    if (!(inodeBitmap[entryInodeNumber / 8] & (1 << (entryInodeNumber % 8)))) {
        delete[] dirData;
        delete[] inodeBitmap;
        delete[] inodes;
        return -ENOTALLOCATED;
    }

    inode_t *entryInode = &inodes[entryInodeNumber];

    // If entry is a directory, check if it is empty
    if (entryInode->type == UFS_DIRECTORY && entryInode->size > 2 * static_cast<int>(sizeof(dir_ent_t))) {
    // Directory is not empty (has more than "." and "..")
    delete[] dirData;
    delete[] inodeBitmap;
    delete[] inodes;
    return -EDIRNOTEMPTY;
    }


    // Free the inode
    inodeBitmap[entryInodeNumber / 8] &= ~(1 << (entryInodeNumber % 8));

    // Free data blocks used by the inode
    unsigned char *dataBitmap = new unsigned char[super.data_bitmap_len * UFS_BLOCK_SIZE];
    readDataBitmap(&super, dataBitmap);

    int numBlocks = (entryInode->size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
    for (int i = 0; i < numBlocks; ++i) {
        unsigned int blockNum = entryInode->direct[i];
        if (blockNum != 0) {
            int dataBlockIndex = blockNum - super.data_region_addr;
            dataBitmap[dataBlockIndex / 8] &= ~(1 << (dataBlockIndex % 8));
            entryInode->direct[i] = 0;
        }
    }

    entryInode->size = 0;

    // Remove directory entry
    dirEntries[entryIndex].name[0] = '\0';  // Mark as unused

    // Check if the block containing the entry is now empty
    int blockIndex = entryIndex / maxEntriesPerBlock;
    bool blockEmpty = true;
    for (int i = 0; i < maxEntriesPerBlock; ++i) {
        if (dirEntries[blockIndex * maxEntriesPerBlock + i].name[0] != '\0') {
            blockEmpty = false;
            break;
        }
    }

    // If block is empty, free it
    if (blockEmpty) {
        unsigned int blockNum = inodes[parentInodeNumber].direct[blockIndex];
        int dataBlockIndex = blockNum - super.data_region_addr;
        dataBitmap[dataBlockIndex / 8] &= ~(1 << (dataBlockIndex % 8));
        inodes[parentInodeNumber].direct[blockIndex] = 0;
        inodes[parentInodeNumber].size -= UFS_BLOCK_SIZE;
    }

    // Write updated directory data back to disk
    for (int i = 0; i < totalBlocks; ++i) {
        unsigned int blockNum = inodes[parentInodeNumber].direct[i];
        if (blockNum != 0) {
            disk->writeBlock(blockNum, &dirEntries[i * maxEntriesPerBlock]);
        }
    }

    // Write updates to disk
    writeInodeBitmap(&super, inodeBitmap);
    writeDataBitmap(&super, dataBitmap);
    writeInodeRegion(&super, inodes);

    delete[] dataBitmap;
    delete[] dirData;
    delete[] inodeBitmap;
    delete[] inodes;

    return 0;
}















        /* Stop if we’ve written all data
        if (blockOffset + writeSize >= size) {
            break;
        }*/