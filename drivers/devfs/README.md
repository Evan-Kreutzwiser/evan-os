# devfs
The devfs driver is a virtual filesystem containing device files created by drivers for interacting with drivers and hardware.

Containing the device files in a separate ramdisk filesystem allows he device files to be retained during the boot process when the root filesystme is mounted and allows the filesystem to be optimized for device files