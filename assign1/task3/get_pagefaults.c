#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmstat.h>

#define PROC_NAME "get_pgfaults"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("total page faults");

static ssize_t pgfaults_read(struct file *file,
                             char __user *buffer,
                             size_t count,
                             loff_t *pos)
{
    char msg[128];
    int len;
    unsigned long pgfaults_count = 0;
    if (*pos > 0)
        return 0;
    
    pgfaults_count = global_node_page_state(PGFAULT);

    len = snprintf(msg, sizeof(msg),
                   "Page Faults: %lu\n",
                   pgfaults_count);

    if (copy_to_user(buffer, msg, len))
        return -1;

    *pos = len;
    return len;
}

static const struct proc_ops pgfaults_proc_ops = {
    .proc_read = pgfaults_read,
};

static int __init custom_init(void)
{
    if (!proc_create(PROC_NAME, 0444, NULL, &pgfaults_proc_ops)) {
        pr_err("Failed to create /proc/%s\n", PROC_NAME);
        return -1;
    }

    pr_info("/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit custom_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(custom_init);
module_exit(custom_exit);
