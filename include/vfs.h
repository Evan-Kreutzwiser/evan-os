/*
 * evan-os/include/vfs.h
 * 
 * Declarations for functions and structures used by the Virtual File System (VFS)
 * This file can also be used by drivers
 */

#ifndef VFS_H
#define VFS_H

#include <stdint.h>

// Error types
#define FS_ERROR_SUCCESS			 0x0 // No error
#define FS_FAILURE					 0x1 // Generic error
#define FS_ERROR_END_OF_FILE		 0x2 // The end of the file was reached
#define FS_ERROR_NULL_BUFFER		 0x3 // Null pointer for buffer
#define FS_ERROR_NULL_FILE  		 0x4 // Null pointer for file
#define FS_ERROR_INVALID_PERMISSIONS 0x5 // The program does not have the right permissions
#define FS_ERROR_DOES_NOT_EXIST      0x6 // The target does not exist

// Inode types
#define FS_FILE        0b00000000 // Regular files
#define FS_DIRECTORY   0b00000001 // Directories
#define FS_CHARDEVICE  0b00000010 // Device files
#define FS_BLOCKDEVICE 0b00000100 // Storage device file
#define FS_PIPE        0b00001000 // Named data pipeline
#define FS_SYMLINK     0b00010000 // Symbolic links
#define FS_MOUNTPOINT  0b00100000 // Active mountpoints

// Define filesystem element types from the structures
typedef struct superblock_t superblock_t;
typedef struct inode_t inode_t;
typedef struct dentry_t dentry_t;

typedef uint32_t (*read_type_t)(dentry_t* file, uint32_t offset, uint32_t size, uint8_t *buffer);
typedef uint32_t (*write_type_t)(dentry_t* file, uint32_t offset, uint32_t size, uint8_t *buffer);
typedef void (*open_type_t)(dentry_t* file, uint8_t read, uint8_t write);
typedef void (*close_type_t)(dentry_t* file);

uint32_t read_fs(dentry_t* file, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(dentry_t* file, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fs(dentry_t* file, uint8_t read, uint8_t write);
void close_fs(dentry_t* file);



// The type used to reference the size of a file
typedef	uint64_t file_size_t;

// Lists filesystem operations that affect he whole filesystem (not only certain files)
typedef struct superblock_operations_t {

	uint64_t (*read) (inode_t*);

	uint64_t (*unmount) (superblock_t*);
	uint64_t (*sync_fs) (superblock_t*);
	uint64_t (*write_inode) (inode_t*, dentry_t*);

} superblock_operations_t;


// Lists file manipulation functions used by filesystem drivers
typedef struct file_operations_t {
	
} file_operations_t;

// Lists inode manipulation functions used by filesystem drivers
typedef struct inode_operations_t {

	uint64_t (*create) (inode_t*, dentry_t*);
	uint64_t (*mkdir) (inode_t*, dentry_t*);
	uint64_t (*rmdir) (inode_t*, dentry_t*);
	uint64_t (*symlink) (inode_t*, dentry_t*);

} inode_operations_t;



// Filesystem superblocks (descriptors)
struct superblock_t {

	char fs_type[8]; // 8 letter filesystem type name

	superblock_operations_t super_ops; // FS
	inode_operations_t		inode_ops; // Inode
	file_operations_t		file_ops;  // Dentry
};

// File nodes
struct inode_t {

	uint32_t 	id;   // The inode's unique id
	uint8_t 	type; // The type of node (See above for types)
	uint64_t	size; // The size of the file in bytes
	uint32_t	link_count; // The number of dentries pointing to this node
	uint64_t*	extra_data; // Points to inode for symbolic links or the superblock 
							// of mount points. Otherwise, should always be 0
};

// File tree entries
struct dentry_t {

	uint32_t	 inode_id;  // The ID of the inode this entry represents
	inode_t*	 inode_ptr; // The memory address of the inode's struct 
	char 		 name[46];  // The name of the entry, as used in file paths
};

#endif // VFS_H