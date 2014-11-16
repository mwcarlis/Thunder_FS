#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#define TUNDER_MAGIC 0x19980122

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


static struct thunder_file_ops(){
	.open = thunder_open,
	.read = thunder_read_file,
	.write = thunder_write_file,
};

struct tree_descr ThunderFiles[] = {
	{NULL, NULL, 0}, // Skipped
	{	.name = "counter0",
		.ops = &thunder_file_ops,
		.mode = S_IWURS | S_IRUGO },
	{	.name = "counter1",
		.ops = &thunder_file_ops,
		.mode = S_IWUSR | S_IRUGO },
	{	.name = "counter2",
		.ops = &thunder_file_ops,
		.mode = S_IWUSR | S_IRUGO },
	{	.name = "counter3",
		.ops = &thunder_file_ops,
		.mode = S_IWUSR | S_IRUGO },
	{ 	"", NULL, 0}
};

// Fill a superblock with stuff
static int thunder_fill_super(struct super_block *sb, void *data, int silent ){
	return simple_fill_super(sb, THUNDER_MAGIC, ThunderFiles);
}

// Stuff to pass in when registering the filesystem
static struct super_block *thunder_get_super(struct file_system_type *fst, 
					int flags, const char *devname, void *data){
	return get_sb_single(fst, flags, lfs_fill_super);
}

static struct file_system_type thunder_type = {
	.owner 		= THIS_MODULE,
	.name 		= "thunder_fs",
	.get_sb		= thunder_get_super,.
	.kill_sb	= kill_litter_super,
};


// Get things up
static int __init thunder_init(void){
	// hangs thunder_type in the chain with head `file_systems`.
	return register_filesystem(&thunder_type);
}

static void __exit thunder_exit(void){
	// unhangs thunder_type from the chain with head `file_systems`.
	unregister_filesystem(&thunder_type);
}

module_init(thunder_init);
module_exit(thunder_exit);