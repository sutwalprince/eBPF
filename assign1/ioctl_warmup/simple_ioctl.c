/*
 * simple_ioctl.c - Simple ioctl driver that returns a constant
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "simple_ioctl"
#define CLASS_NAME "simple"

/* Define ioctl commands */
#define IOCTL_MAGIC 'S'
#define IOCTL_GET_CONSTANT _IOR(IOCTL_MAGIC, 1, int)

/* The constant we'll return */
#define MY_CONSTANT 499

static int major_number;
static struct class *device_class = NULL;
static struct device *device_driver = NULL;
static struct cdev my_cdev;

/* Function prototypes */
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);

/* File operations structure */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

/* Called when device is opened */
static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "simple_ioctl: Device opened\n");
    return 0;
}

/* Called when device is closed */
static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "simple_ioctl: Device closed\n");
    return 0;
}

/* ioctl handler */
static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int value;
    int __user *user_ptr = (int __user *)arg;

    printk(KERN_INFO "simple_ioctl: ioctl called with cmd=%u\n", cmd);

    if (cmd == IOCTL_GET_CONSTANT)
    {
        value = MY_CONSTANT;
        if (copy_to_user(user_ptr, &value, sizeof(int)))
        {
            printk(KERN_ALERT "simple_ioctl: Failed to copy to user\n");
            return -EFAULT;
        }
        printk(KERN_INFO "simple_ioctl: Returned constant %d\n", value);
        return 0;
    }
}

/* Module initialization */
static int __init simple_ioctl_init(void)
{
    dev_t dev;
    int ret;

    printk(KERN_INFO "simple_ioctl: Initializing module\n");

    /* Allocate major number dynamically */
    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ALERT "simple_ioctl: Failed to allocate major number\n");
        return ret;
    }
    major_number = MAJOR(dev);
    printk(KERN_INFO "simple_ioctl: Registered with major number %d\n", major_number);

    /* Initialize cdev structure */
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    /* Add character device to the system */
    ret = cdev_add(&my_cdev, dev, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "simple_ioctl: Failed to add cdev\n");
        return ret;
    }

    /* Create device class */
    device_class = class_create(CLASS_NAME);
    if (IS_ERR(device_class))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "simple_ioctl: Failed to create class\n");
        return PTR_ERR(device_class);
    }

    /* Create device file */
    device_driver = device_create(device_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(device_driver))
    {
        class_destroy(device_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "simple_ioctl: Failed to create device\n");
        return PTR_ERR(device_driver);
    }

    printk(KERN_INFO "simple_ioctl: Device created successfully\n");
    return 0;
}

/* Module cleanup */
static void __exit simple_ioctl_exit(void)
{
    dev_t dev = MKDEV(major_number, 0);

    device_destroy(device_class, dev);
    class_destroy(device_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "simple_ioctl: Module removed\n");
}

module_init(simple_ioctl_init);
module_exit(simple_ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("Simple ioctl driver");
MODULE_VERSION("1.0");