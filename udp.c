#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Noam Goldgraber");
MODULE_DESCRIPTION("Registers a device which allows user to send packet through.");

static int __init ModuleInit(void) {
    printk(KERN_INFO "UDP Module Initiating!\n");
    return 0;
}

static void __exit ModuleExit(void) {
    printk(KERN_INFO "UDP Module cleaned-up!\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);