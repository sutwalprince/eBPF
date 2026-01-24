#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <asm/pgtable.h>
#include <asm/io.h>

#define DEVICE_NAME "ioctl_earth"
#define CLASS_NAME "ioctl"

#define IOCTL_MAGIC 'P'
#define IOCTL_CHANGE_PARENT _IO(IOCTL_MAGIC, 1)
#define IOCTL_TERMINATE_CHILDREN _IOW('P', 2, pid_t)


static pid_t control_centre_pid = 1;
module_param(control_centre_pid, int, 0);
MODULE_PARM_DESC(control_centre_pid, "Process ID of control centre");

static int major_number;
static struct class *device_class = NULL;
static struct device *device_driver = NULL;
static struct cdev my_cdev;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Task2.2: Device opened by PID %d\n", current->pid);
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Task2.2: Device closed by PID %d\n", current->pid);
    return 0;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{

    struct task_struct *task_child = current;
    struct task_struct *task_contol_center;
    task_contol_center = pid_task(find_vpid(control_centre_pid), PIDTYPE_PID);
    if (task_contol_center == NULL )
    {
        printk(KERN_ERR "Task2.2: Control centre process with PID %d not found\n", control_centre_pid);
        return -ESRCH;
    }

    printk(KERN_INFO "Task2.2: ioctl called with cmd=0x%x by PID %d\n",
           cmd, current->pid);

    if (cmd == IOCTL_CHANGE_PARENT)

    {

        printk(KERN_INFO "Task2.2: Soldier pid %d\n", current->pid);
        printk(KERN_INFO "Task2.2: soldier real parent pid %d\n", task_child->real_parent->pid);
        printk(KERN_INFO "Task2.2: soldier parent pid %d\n", task_child->parent->pid);
        printk(KERN_INFO "Task2.2: Changing parent to PID %d\n", control_centre_pid);

        
        list_del_init(&task_child->sibling);

        task_child->parent = task_contol_center;
        task_child->real_parent = task_contol_center;
        list_add_tail(&task_child->sibling, &task_contol_center->children);
        
        task_child->exit_signal = SIGCHLD;

        if (task_child->parent == task_contol_center && task_child->real_parent == task_contol_center)
        {
            printk(KERN_INFO "Task2.2: Parent changed successfully\n");
        }
        else
        {
            printk(KERN_ERR "Task2.2: Failed to change parent\n");
        }
    }
    if(cmd == IOCTL_TERMINATE_CHILDREN)
    {
        pid_t parent_pid;
        if (copy_from_user(&parent_pid, (pid_t __user *)arg, sizeof(pid_t)))
        {
            printk(KERN_ERR "Task2.2: Failed to copy parent PID\n");
            return -1;
        }

        struct task_struct *parent_task;
        struct task_struct *child_task;
        parent_task = pid_task(find_vpid(parent_pid), PIDTYPE_PID);
        if (parent_task == NULL)
        {
            printk(KERN_ERR "Task2.2: Parent process not found\n");
            return -1;
        }

        list_for_each_entry(child_task, &parent_task->children, sibling)
        {
            printk(KERN_INFO "Task2.2: Terminating child PID %d of parent PID %d\n", child_task->pid, parent_pid);
            send_sig(SIGTERM, child_task, 0);
        }
    }

    return 0;
}

static int __init ioctl_mapper_init(void)
{
    dev_t dev;
    int ret;

    printk(KERN_INFO "Task2.2: Initializing module\n");

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "Task2.2: Failed to allocate major number\n");
        return ret;
    }
    major_number = MAJOR(dev);
    printk(KERN_INFO "Task2.2: Registered with major number %d\n", major_number);

    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, dev, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Task2.2: Failed to add cdev\n");
        return ret;
    }

    device_class = class_create(CLASS_NAME);
    if (IS_ERR(device_class))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Task2.2: Failed to create class\n");
        return PTR_ERR(device_class);
    }

    device_driver = device_create(device_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(device_driver))
    {
        class_destroy(device_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Task2.2: Failed to create device\n");
        return PTR_ERR(device_driver);
    }

    printk(KERN_INFO "Task2.2: Device created successfully at /dev/%s\n",
           DEVICE_NAME);
    return 0;
}

static void __exit ioctl_mapper_exit(void)
{
    dev_t dev = MKDEV(major_number, 0);

    device_destroy(device_class, dev);
    class_destroy(device_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Task2.2: Module removed\n");
}

module_init(ioctl_mapper_init);
module_exit(ioctl_mapper_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("parent change using ioctl");
