/* 
 * evan-os/drivers/devfs/devfs.c
 * 
 * A ramdisk filesystem driver containing all of the device files used by 
 * drivers to allow drivers to interact with each other and 
 * allow software to use hardware.
 * 
 */

#include <vfs.h>


operations_t __attribute__((align(64))) operations = {

	.mount = devfs_mount,

};


uint64_t devfs_mount(char* mount_directory) {

	// TODO: Find node to mount on with syscall
	inode_t mount_point;

	// Check if the mount point is a directory
	if (mount_point.type != FS_DIRECTORY) {
		return FS_ERROR_NODE_TYPE;
	}

	// TODO: Use syscall to mount

	return FS_ERROR_SUCCESS;
}
