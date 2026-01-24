#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

// Module metadata
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("Peeking into Linux processes");
MODULE_LICENSE("GPL");

static int pid = 0;
module_param(pid, int, S_IRUGO);
MODULE_PARM_DESC(pid, "Process ID");

static int __init custom_init(void)
{
    struct task_struct *task;

    printk(KERN_INFO "Printing child processes of PID %d.\n", pid);
    for_each_process(task)
    {
        if (task->parent->pid == pid)
        {
            printk(KERN_INFO "PID: %d  Name: %s  State: %ld\n",
                   task->pid, task->comm, task->__state);
        }
    }
    printk(KERN_INFO "All child processes of PID %d printed.\n", pid);
    return 0;
}
static void __exit custom_exit(void)
{
    printk(KERN_INFO "Unloading Module.\n");
}
module_init(custom_init);
module_exit(custom_exit);