#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

// Module metadata
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("Peeking into Linux ");
MODULE_LICENSE("GPL");
// Custom init and exit methods
static int __init custom_init(void)
{
    struct task_struct *task;

    printk(KERN_INFO "Printing all processes.");
    for_each_process(task)
    {
        if (task->__state == TASK_RUNNING)
        {
            printk(KERN_INFO "PID: %d | Name: %s\n",
                   task->pid, task->comm);
        }
    }
    printk(KERN_INFO "All processes printed.");
    return 0;
}
static void __exit custom_exit(void)
{
    printk(KERN_INFO "Unloading Module.");
}
module_init(custom_init);
module_exit(custom_exit);