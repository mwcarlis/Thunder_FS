
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


########### BUILD Socket ###########
The socket is built into the thunderfs.c file.
They build together.  Just $make in the top level
of the directory structure and all kernel modules
are built.  

To build the user space filesystem.
> user_thunderfs/user_thunderfs.c is built into this.
> The Makefile does not check the timestamp of 
	user_thunderfs/user_thunderfs.c before the build.

1. $ cd socket
2. $ make user_thunder.o

1. You must load thunderfs.ko into the kernel.
2. $ tail -f /var/log/syslog
3. $ ./user_thunder.o


########### MOUNT / UNMMOUNT Module  ###########
To mount:	$ sudo mount -v -t thunderfs /dev/ram0 dir_test
Unmount:	$ sudo unmount -v dir_test


###########	UNIT TESTS ###########
<<<<<<< HEAD
=======
All unit tests depend on a the directory Thunder_FS/dir_test 
$ mkdir ./dir_test
>>>>>>> Initial commit of test_file_mkrm.py

-- Mount Test:
Use $ sudo ./test_mount.py 
	to test 100 mount/umount sequences
<<<<<<< HEAD
=======

-- File Creation/Removal Test:
Use $ sudo ./test_file_mkrm.py
	to test 100 touch/rm sequences
>>>>>>> Initial commit of test_file_mkrm.py
