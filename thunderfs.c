
//#define MODULE
//#define LINUX
//#define __KERNEL__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include "./socket/thunderlink.c"

#define THUNDER_MAGIC 0x69FF69FF
static int thunder_mknod(struct inode *dir, struct dentry *dentry, 
                                                umode_t mode, dev_t dev);

static int thunder_implode_inode(struct inode *thisnode){
        generic_delete_inode(thisnode);
        printk(KERN_INFO "thunder_implode_inode\n");
        return 0;
}

static struct super_operations thunder_s_ops = {
        .statfs         = simple_statfs,
        .drop_inode     = thunder_implode_inode,
        .show_options   = generic_show_options,
};

static ssize_t thunder_read(struct file *filp, char __user *buf, size_t count, loff_t *offset){
        thunder_cmd_dispatch(DISPATCH_READ, -1);
        printk(KERN_INFO "thunder_read\n");
        return 0;
}

static ssize_t thunder_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset){
        thunder_cmd_dispatch(DISPATCH_WRITE, -1);
        printk(KERN_INFO "thunder_write\n");
        return -EPERM;
}

static int thunder_open(struct inode *inod, struct file* filp){
        unsigned long inode_id = inod->i_ino;
        thunder_cmd_dispatch(DISPATCH_OPEN, inode_id);
        printk(KERN_INFO "thunder_open\n");
        return 0;
}

const struct file_operations thunder_file_operations = {
        .read           = thunder_read,
        .write          = thunder_write,
        .open           = thunder_open,
        .fsync          = noop_fsync,
};

const struct inode_operations thunder_file_inode_operations = {
        .setattr        = simple_setattr,
        .getattr        = simple_getattr,
};


// --------------------------------------- dir_inode_operations
static int thunder_create(struct inode *dir, struct dentry *dentry, 
                                                umode_t mode, bool excl){
        return thunder_mknod(dir, dentry, mode | S_IFREG, 0);
        printk(KERN_INFO "Returning thunder_create\n");
}

static int thunder_rmdir(struct inode *dir, struct dentry *dentry){
        simple_rmdir(dir, dentry);
        printk(KERN_INFO "Returning thunder rmdir\n");
        return 0;
}

const struct inode_operations thunder_dir_inode_operations = {
        .create         = thunder_create,
        .mknod          = thunder_mknod,
        .link           = simple_link,
        .lookup         = simple_lookup,
        .unlink         = simple_unlink,
        .rename         = simple_rename,
        .rmdir          = thunder_rmdir,
};

// ---------------------------------------

static struct inode *thunder_make_inode(struct super_block *sb, 
                const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode *ret = new_inode(sb);
	if (ret) {
                ret->i_ino = get_next_ino();
		ret->i_mode = mode;
                inode_init_owner(ret, dir, mode);
		//i_uid_write(ret, (uid_t) 0);
		//i_gid_write(ret, (gid_t) 0);
		ret->i_blocks = 0;
		ret->i_atime = ret->i_mtime = ret->i_ctime = CURRENT_TIME;
                switch(mode & S_IFMT){
                        case S_IFREG:
                                ret->i_op = &thunder_file_inode_operations;
                                ret->i_fop = &thunder_file_operations;
                                printk(KERN_INFO "Creating File Inode\n");
                                break;
                        case S_IFDIR:
                                ret->i_op = &thunder_dir_inode_operations;
                                ret->i_fop = &simple_dir_operations;
                                inc_nlink(ret);
                                printk(KERN_INFO "Creating Directory Inode\n");
                                break;
                        default:
                                printk(KERN_INFO "Default Case of Create Inode\n");
                                break;
                }
		printk(KERN_INFO "Succesfully create an inode\n");
	}
	return ret;
}

// Create  a File
static int thunder_mknod(struct inode *dir, struct dentry *dentry, 
                        umode_t mode, dev_t dev){

        struct inode *ret = thunder_make_inode(dir->i_sb, dir, mode, dev);
        int error = -ENOSPC;
        if(ret){
                d_instantiate(dentry, ret);
                dget(dentry);   /* Extra count - pin the dentry in core */
                error = 0;
                dir->i_mtime = dir->i_ctime = CURRENT_TIME;
        }
        printk(KERN_INFO "Return thunder_mknod\n");
        return error;
}


static int thunder_fill_super(struct super_block *sb, void *data, int silent){
        struct inode *root;

        save_mount_options(sb, data);
        //sb->s_maxbytes = MAX_LFS_FILESIZE;
        sb->s_magic = THUNDER_MAGIC;
        sb->s_op = &thunder_s_ops;
        sb->s_time_gran = 1;

        root = thunder_make_inode(sb, NULL, S_IFDIR | 0755, 0);

        sb->s_root = d_make_root(root);
        if(!sb->s_root){
                return -ENOMEM;
        }

        printk(KERN_INFO "Fill Super\n");
        return 0;
}

struct dentry *thunder_mount(struct file_system_type *fst,
                        int flags, const char *dev_name, void *data){
        printk(KERN_INFO "thunder_mount\n");
        //printk(KERN_INFO "dv_nm: %c" , dev_name);
        //printk(KERN_INFO "", );
        return mount_nodev(fst, flags, data, thunder_fill_super);
}

static void thunder_kill_sb(struct super_block *sb){
        kill_litter_super(sb);
        printk(KERN_INFO "thunder_kill_sb\n");
}

static struct file_system_type thunder_type = {
        .name           = "thunderfs",
        .mount          = thunder_mount,
        .kill_sb        = thunder_kill_sb,
        .fs_flags       = FS_USERNS_MOUNT,
};

int init_socket(void)
{
        int rc;
        struct genl_family *init = &thunder_gnl_family;
        struct genl_ops *init_ops = thunder_gnl_ops;
        init->ops = init_ops;
        printk(KERN_INFO "T_A_MAX: %i\n", THUNDER_A_MAX);
        init->n_ops = 4;
        rc = genl_register_family(init);
        if( rc != 0 ){
                printk(KERN_INFO "Register Ops: %i\n", rc);
                genl_unregister_family(init);
                printk(KERN_INFO "An Error Occured with insmod\n");
                return -1;
        }
        return 0;

}

int __init init_mod(void) // Required to insmod
{
        static unsigned long once;
        int rv = init_socket();
        if(rv != 0 ){
                return -1;
        }
        if (test_and_set_bit(0, &once))
                return 0;

        printk(KERN_INFO "Hello Cruel World\n");
        return register_filesystem(&thunder_type);

}

void clean_mod(void) // Required for rmmod
{
        int ret;
        ret = genl_unregister_family(&thunder_gnl_family);
        if(ret != 0 ){
                printk(KERN_INFO "An Error Unregistering Family\n");

        }
        ret = unregister_filesystem(&thunder_type);
        if(ret != 0 ){
                printk(KERN_INFO "An Error Unregistering FileSystem\n");

        }
        printk(KERN_INFO "Goodbye Cruel World\n");
}

module_init(init_mod);
module_exit(clean_mod);

MODULE_AUTHOR("Matthew Carlis");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Starting fresh");

