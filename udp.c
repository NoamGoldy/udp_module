#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define MAJOR_NR (27)
#define MINOR_NR (01)
#define MTU (1500)
#define DRIVER_NAME ("udp_device")
#define DRIVER_CLASS ("MyModuleClass")

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noam Goldgraber");
MODULE_DESCRIPTION("Registers a device which allows user to send packet through.");

static char packet_bf[MTU];
static int buffer_pointer;

static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;

static int device_open(struct inode* device_file, struct file* instance) {
    printk("dev_nr - open was called!\n");
    return 0;
}

static int device_close(struct inode* device_file, struct file* instance) {
    printk("dev_nr - close was called!\n");
    return 0;
}

static ssize_t device_write(struct file* f, const char* user_buffer, size_t count, loff_t* offs) {
	int to_copy, not_copied;
	printk("dev_nr - write was called!\n");

	to_copy = min(count, sizeof(packet_bf));
	not_copied = copy_from_user(packet_bf, user_buffer, to_copy);
	buffer_pointer = to_copy;

	printk(packet_bf);

	return to_copy - not_copied;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.write = device_write
};

static int __init ModuleInit(void) {
    printk(KERN_INFO "UDP Module Initiating!\n");

	/* Allocating device nr*/
	if( alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
		printk("Device Nr. could not be allocated!\n");
		return -1;
	}
	printk("udp_device Nr. Major: %d, Minor: %d was registered!\n", my_device_nr >> 20, my_device_nr & 0xfffff);

	if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}

	return 0;
	
AddError:
	device_destroy(my_class, my_device_nr);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(my_device_nr, 1);
	return -1;
    
}

static void __exit ModuleExit(void) {
	cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_nr, 1);

	printk(KERN_INFO "UDP Module Exiting!\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);