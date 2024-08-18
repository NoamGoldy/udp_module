#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#define MAJOR_NR (27)
#define MINOR_NR (01)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noam Goldgraber");
MODULE_DESCRIPTION("Registers a device which allows user to send packet through.");

static int device_open(struct inode* device_file, struct file* instance) {
    printk("dev_nr - open was called!\n");
    return 0;
}

static int device_close(struct inode* device_file, struct file* instance) {
    printk("dev_nr - close was called!\n");
    return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close
};

static int __init ModuleInit(void) {
	int retval;
    printk(KERN_INFO "UDP Module Initiating!\n");
    retval = register_chrdev(MAJOR_NR, "my_dev_nr", &fops);
    if(retval == 0) {
		printk("dev_nr - registered Device number Major: %d, Minor: %d\n", MAJOR_NR, 0);
	}
	else if(retval > 0) {
		printk("dev_nr - registered Device number Major: %d, Minor: %d\n", retval>>20, retval&0xfffff);
	}
	else {
		printk("Could not register device number!\n");
		return -1;
	}
	return 0;
}

static void __exit ModuleExit(void) {
    unregister_chrdev(MAJOR_NR, "my_dev_nr");
    printk(KERN_INFO "UDP Module cleaned-up!\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);