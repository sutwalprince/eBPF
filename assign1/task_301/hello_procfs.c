#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "hello_procfs"
#define MESSAGE "Hello World!\n"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("Hello World Procfs Module");

static ssize_t hello_proc_read(struct file *file,
                               char __user *buffer,
                               size_t count,
                               loff_t *pos)
{
    int len = strlen(MESSAGE);

    if (*pos > 0)
        return 0;

    if (copy_to_user(buffer, MESSAGE, len))
        return -EFAULT;

    *pos = len;
    return len;
}

static const struct proc_ops hello_proc_ops = {
    .proc_read = hello_proc_read,
};

static int __init hello_procfs_init(void)
{
    proc_create(PROC_NAME, 0444, NULL, &hello_proc_ops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit hello_procfs_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(hello_procfs_init);
module_exit(hello_procfs_exit);
