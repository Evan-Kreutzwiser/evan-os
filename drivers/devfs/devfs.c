/* 
 * evan-os/drivers/devfs/devfs.c
 * 
 * A ramdisk filesystem driver containing all of the device files used by 
 * drivers to allow drivers to interact with each other and 
 * allow software to use hardware.
 * 
 */

#include <vfs.h>


inode_operations_t __attribute__((align(64))) inode_operations = {

	.create = super_create,

};


void super_create(inode_t* inode, dentry_t* dentry) {

}


// TODO: Include permissions
int devfs_mkdir(char* name) {


};