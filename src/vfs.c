/*
 * evan-os/src/vfs.c
 * 
 * Setting up and managing the virtual file system
 * 
 */

#include <vfs.h>

#include <stdint.h>


dentry_t* fs_root = (dentry_t*)0x0;

inode_t* mount_roots[30];  // The root diredctory of mounted devices
						   // Allows the devices to be mounted based on inode id when file paths 
						   // are not available
inode_t* mount_points[30]; // Where devices are mounted

uint64_t read_fs(dentry_t* file, uint64_t offset, uint64_t size, uint8_t* buffer) {

	// Find the inode of the passed dentry
	inode_t* inode = (file->inode_ptr);

	// If the inode pointer is null
	if (inode == 0)       { return FS_ERROR_NULL_BUFFER; }
	// If the buffer pointer is null
	else if (buffer == 0) { return FS_ERROR_NULL_BUFFER; }
	// If the caller requested 8 bytes
	else if (size == 0)   { return FS_ERROR_FAILURE; }
	// If the offset is outside the file size
	// Remove when real file reading is implemenet
	else if (inode->size < offset) { return FS_ERROR_END_OF_FILE; }

	// Use the filesystem's functions to read the file into the buffer

	// TODO: implement file reading

	// Return the success
	return FS_ERROR_SUCCESS;
}

uint64_t mount_root(char* file) {

	// If the string is null or 0 length
	if (file == 0 || file[0] == '\0') {
		return FS_ERROR_INVALID_PATH;
	}

	// Return an error, since mounting is not supported yet
	return FS_ERROR_FAILURE;
}