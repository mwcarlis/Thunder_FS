#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>


// Boilerplate 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matthew Carlis, Duc Tran, /*Add-in names here*/");

// ***
//	Operations on the files in the New Thunder_FS.
// ***

// Open a file
static int thunder_open(struct inode *inode, struct file *flip){
}

// Read a file.
static ssize_t thunder_read_file(struct file *flip, char *buf,
				size_t count, loff_t *offset){
}

// Write a file
static ssize_t thunder_write_file(struct file *flip, const char *buf,
				size_t count, loft_t *offset){
}


static struct file_operations(){
	.open = thunder_open,
	.read = thunder_read_file,
	.write = thunder_write_file,
};

// Fill a superblock with stuff
static int thunder_fill_super(struct file_system_type *fst,
				int flags, const char *devname, void *data){
	
}

// Stuff to pass in when registering the filesystem
static struct super_block *thunder_get_super(struct file_system_type *fst, 
					int flags, const char *devname, void *data){
}

static struct file_system_type thunder_type = {
	.owner 		= THIS_MODULE,
	.name 		= "thunder_fs",
	.get_sb		= thunder_get_super,.
	.kill_sb	= kill_litter_super,
};


// Get things up
static int __init thunder_init(void){
}

static void __exit thunder_exit(void){
	unregister_filesystem(&thunder_type);
}

module_init(thunder_init);
module_exit(thunder_exit);