# Distributed-File-System-Project
Project Overview
This project implements a Distributed File System (DFS) that combines core file system operations with a RESTful HTTP interface. The system enables efficient management of files and directories stored on a disk image, following Unix File System (UFS) principles.

Background
The main idea behind a distribute file system is that you can have multiple clients access the same file system at the same time. One popular distributed file system, which serves as inspiration for this project, is Amazon's S3 storage system. S3 is used widely and provides clear semantics with a simple REST/HTTP API for accessing data. With these basics in place, S3 provides the storage layer that powers many of the modern apps we all use every day.

Like local file systems, distributed file systems support a number of high level file system operations. In this project you'll implement read(), write(), and delete() operations on objects using HTTP APIs.

HTTP/REST API
two different entities: files and directories. You will access these entities using standard HTTP/REST network calls. All URL paths begin with /ds3/, which defines the root of your file system.

To create or update a file, use HTTP PUT method where the URL defines the file name and path and the body of your PUT request contains the entire contents of the file. If the file already exists, the PUT call overwrites the contents with the new data sent via the body.

In the system, directories are created implicitly. If a client PUTs a file located at /ds3/a/b/c.txt and directories a and b do not already exist, you will create them as a part of handling the request. If one of the directories on the path exists as a file already, like /a/b, then it is an error.

To read a file, you use the HTTP GET method, specifying the file location as the path of your URL and your server will return the contents of the file as the body in the response. To read a directory, you also use the HTTP GET method, but directories list the entries in a directory. To encode directory entries, you put each directory entry on a new line and regular files are listed directly, and directories are listed with a trailing "/". For GET on a directory omit the entries for . and ... For example, GET on /ds3/a/b will return:

c.txt

And GET on /ds3/a/ will return:

b/

The listed entries should be sorted using standard string comparison sorting functions.

To delete a file, you use the HTTP DELETE method, specifying the file location as the path of your URL. To delete a directory, you also use DELETE but deleting a directory that is not empty it is an error.

You will implement your API handlers in DistributedFileSystemService.cpp.

Since Gunrock is a HTTP server, you can use command line utilities, like cURL to help test it out. Here are a few example cURL command:

% curl -X PUT -d "file contents" http://localhost:8080/ds3/a/b/c.txt 
% curl http://localhost:8080/ds3/a/b/c.txt                          
file contents
% curl http://localhost:8080/ds3/a/b/     
c.txt
% curl http://localhost:8080/ds3/a/b 
c.txt
% curl http://localhost:8080/ds3/a  
b/
% curl -X DELETE http://localhost:8080/ds3/a/b/c.txt
% curl http://localhost:8080/ds3/a/b/               
% 


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
Name: Yanming
Email: luoyanming99@gmail.com
Date: 12/17/2024
License: Open-source under MIT License.
