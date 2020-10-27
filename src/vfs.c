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

	// If there is a read function
	//if (inode->read != 0x0) {
		// Read the file with the correct read function and pass on the return value
		//return inode->read(file, offset, size, buffer);
	//}
	// If there is no function
	//else {
		// Indicate failure
		return 0x1;
	//}
}

uint32_t mount_root(char* file) {

	return 1;
}