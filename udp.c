#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/netdevice.h>
#include <linux/fs.h>

#define DEFAULT_PORT 10001
#define CONNECT_PORT 10002
#define INADDR_SEND (0xac1fb03c) /* My host IP */
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

struct mymsghdr {
    void *msg_name;		/* Address to send to/receive from.  */
    size_t msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    size_t msg_iovlen;		/* Number of elements in the vector.  */

    void *msg_control;		/* Ancillary data (eg BSD filedesc passing). */
    size_t msg_controllen;	/* Ancillary data buffer length.
				   !! The type should be socklen_t but the
				   definition of the kernel is incompatible
				   with this.  */

    int msg_flags;		/* Flags on received message.  */
  };

int ksocket_send(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len)
{
        struct mymsghdr msg;
        struct iovec iov;
        int size = 0;

        if (sock->sk == NULL) {
           return 0;
		}

        iov.iov_base = buf;
        iov.iov_len = len;

        msg.msg_flags = 0;
        msg.msg_name = addr;
        msg.msg_namelen  = sizeof(struct sockaddr_in);
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        // -- Now leads to WSL panic -- size = sock_sendmsg(sock,(struct msghdr *)&msg);

        return size;
}

static void send_udp_message(void) {
	// creating socket and binding
	struct socket* sock;
	struct socket* sock_send;
	sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
	sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock_send);
	printk("Sock is %lu\n", (unsigned long)sock);

	char buf[10] = "Hello!\n";
	int len = 10;
	struct sockaddr_in addr, addr_send;

	addr.sin_family = AF_INET;
	addr_send.sin_addr.s_addr = htonl(INADDR_SEND);

	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_send.sin_addr.s_addr = htonl(INADDR_SEND);

	addr.sin_port = htons(DEFAULT_PORT);
	addr_send.sin_port = htons(CONNECT_PORT);

	sock->ops->bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr));

	sock_send->ops->connect(sock_send, (struct sockaddr *)&addr_send, sizeof(struct sockaddr), 0);

	printk("Listening on port %d\n", DEFAULT_PORT);

	ksocket_send(sock_send, &addr_send, buf, len);

	sock_release(sock);
	sock_release(sock_send);
	sock = 0;
}

static int __init ModuleInit(void) {
	send_udp_message();

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