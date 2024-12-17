Project Overview
This project implements a Distributed File System (DFS) that combines core file system operations with a RESTful HTTP interface. The system enables efficient management of files and directories stored on a disk image, following Unix File System (UFS) principles.

Features
Local File System Layer:

File and directory operations: create, read, write, and delete.
Supports block-based data storage using inodes and data blocks.
Efficiently handles small and large files with block chaining and hierarchical structure.
HTTP Service Layer:

GET: Retrieve file contents or list directory entries.
PUT: Create or update files and directories.
DELETE: Remove files or directories with proper validation.
RESTful API interface implemented in DistributedFileSystemService.cpp.
Command-Line Utilities:

ds3ls: List contents of a directory.
ds3cp: Copy a file from the host system to the disk image.
ds3rm: Remove a file or directory.
ds3mkdir: Create a directory.
ds3bits: Display metadata like superblock, inode, and data bitmaps.
File Operations:

Reads and writes data in 4 KB blocks (UFS_BLOCK_SIZE).
Supports file movement across directories using move().
Files and Key Components
LocalFileSystem.cpp: Core file system implementation for file and directory management.
DistributedFileSystemService.cpp: HTTP service layer for distributed file system operations.
disk.cpp: Low-level disk read and write operations.
Command-Line Tools:
ds3ls, ds3cp, ds3rm, ds3mkdir, ds3bits: Utilities for file system interactions.
HTTPRequest/HTTPResponse.cpp: Handles HTTP requests and responses for client-server communication.
HttpUtils.cpp: Helper functions for query parsing, HTTP chunking, and string utilities.
How to Build and Run
Build the Project:

Use the provided Makefile to compile all source files:
bash

make
Run the HTTP Server:

Start the Distributed File System service:
bash

./server <disk_image_file>
Use Command-Line Utilities:

Example commands:
List directory contents:
bash

./ds3ls disk.img /home
Copy a file into the disk:
bash

./ds3cp disk.img hostfile.txt 3
Remove a file:
bash

./ds3rm disk.img 2 file.txt
Interact via HTTP:

Create or update a file:
bash

curl -X PUT -d "Hello World!" http://localhost:8080/ds3/a/b/c.txt
Retrieve file contents:
bash

curl http://localhost:8080/ds3/a/b/c.txt
Delete a file:
bash

curl -X DELETE http://localhost:8080/ds3/a/b/c.txt
Technologies Used
Programming Language: C++
HTTP Framework: Custom HTTP parser and response generator
File System: Unix File System (UFS) principles with block-based storage
Tools: make, curl, command-line utilities
Future Improvements
Add caching for frequently accessed data.
Implement triple-indirect pointers for extremely large files.
Add support for concurrent operations with locking mechanisms.
Author
Name: Yanming Luo
Email: luoyanming99@gmail.com
Date: 12/17/2024
License: Open-source under MIT License.
