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

uint32_t read_fs(dentry_t* file, uint32_t offset, uint32_t size, uint8_t *buffer) {

	// Find the inode of the passed dentry
	inode_t* inode = (file->inode_ptr);

	// If he buffer pointer
	if (buffer == 0)    { return FS_ERROR_NULL_BUFFER; }
	else if (size == 0) { return FS_ERROR_FAILURE; }

	// Use the filesystem's functions to read the file into the buffer

	// TODO: implement file reading

	// Return the success
	return FS_ERROR_SUCCESS;
}

uint32_t mount_root(char* file) {

	return 1;
}