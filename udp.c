#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

int init_module(void) {
    printk(KERN_INFO "UDP Module YOOOO!\n");
    return 0;
}

void cleanup_module(void) {
    printk(KERN_INFO "UDP Module cleaned-up!\n");
}