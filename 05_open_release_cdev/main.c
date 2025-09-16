#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

static int major;


static int my_open(struct inode* inode, struct file* filep) {
	// inode is used to represent a file and we can get the major and minor device number for the file
	// filep represents a open file in linux kernel, created after calling the open function and destroyed after calling the release function
	pr_info("hello_cdev - Major %d, Minor %d\n", imajor(inode), iminor(inode)); 

	pr_info("hello_cdev - current file position %lld\n", filep->f_pos);
	pr_info("hello_cdev - current fmode 0x%x\n", filep->f_mode);
	pr_info("hello_cdev - current file flags 0x%x\n", filep->f_flags);
	return 0;
}

static int my_release(struct inode* inode, struct file* filep) {
	pr_info("bye_cdev - current file closed with Major: %d, Minor %d\n", imajor(inode), iminor(inode));
	return 0;
}

static struct file_operations fops = {
	.open = my_open,
	.release = my_release
};

static int __init my_init(void)
{
	major = register_chrdev(0, "hello_cdev", &fops);
	if (major < 0) {
		pr_err("hello_cdev - Error registering chrdev\n");
		return major;
	}
	pr_info("hello_cdev - Major Device Number: %d\n", major);
	return 0;
}

static void __exit my_exit(void)
{
	unregister_chrdev(major, "hello_cdev");
	pr_notice("bye_cdev - Mjor Device Number : %d\n", major);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4Linux");
MODULE_DESCRIPTION("A sample driver for registering a character device");
