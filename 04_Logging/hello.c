#include <linux/module.h>
#include <linux/init.h>

static int __init my_init(void) {
	printk(KERN_INFO "log level - This is info log\n");
	printk(KERN_WARNING "log level - This is warning log\n");
	printk(KERN_ALERT "log level - This is alert log\n");
	printk(KERN_DEBUG "log level - This is debug log\n");
	return 0;
}

static void __exit my_exit(void) {
	pr_emerg("log level - Module is unloaded\n");
	pr_err("log level - Error!\n");
	pr_info("log level - final module message\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRATEEK");
MODULE_DESCRIPTION("Simple hello world driver");
