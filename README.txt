

########### BUILD / LOAD MODULE ###########
To build the source: 	$ make
To clean the build: 	$ make clean
To insert in kernel:	$ sudo insmod thunderfs
Remove from Kernel:		$ sudo rmmod thunderfs

TRAPS:
When you create the directory and mount our fs.  Check
$ ls -l    : For     root root.  
This means you must be root user to modify this directory.

To Enter Root:
$ sudo -i



########### MOUNT / UNMMOUNT Module  ###########
To mount:				$ sudo mount -v -t thunderfs /dev/ram0 dir_test
Unmount:				$ sudo unmount -v dir_test



###########	UNIT TESTS ###########

-- Mount Test:
Use $ sudo ./test_mount.py 
	to test 100 mount/umount sequences
