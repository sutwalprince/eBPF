
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>

static int pid = -1;
static char unit = 'B';
static struct kobject *mem_stats_kobj;

static ssize_t pid_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf) {
    return sprintf(buf, "%d\n", pid);
}

static ssize_t pid_store(struct kobject *kobj, struct kobj_attribute *attr,
                         const char *buf, size_t count) {
    sscanf(buf, "%d", &pid);
    return count;
}

static ssize_t unit_show(struct kobject *kobj, struct kobj_attribute *attr,
                         char *buf) {
    return sprintf(buf, "%c\n", unit);
}

static ssize_t unit_store(struct kobject *kobj, struct kobj_attribute *attr,
                          const char *buf, size_t count) {
    if (buf[0] == 'B' || buf[0] == 'K' || buf[0] == 'M') {
        unit = buf[0];
    }
    return count;
}

static ssize_t virtmem_show(struct kobject *kobj, struct kobj_attribute *attr,
                            char *buf) {
    struct task_struct *task;
    struct mm_struct *mm;
    unsigned long virtmem = 0;

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task) {
        mm = task->mm;
        if (mm) {
            virtmem = mm->total_vm << PAGE_SHIFT;
        }
    }
    rcu_read_unlock();

    switch (unit) {
    case 'K':
        virtmem >>= 10;
        break;
    case 'M':
        virtmem >>= 20;
        break;
    }

    return sprintf(buf, "%lu\n", virtmem);
}

static ssize_t physmem_show(struct kobject *kobj, struct kobj_attribute *attr,
                            char *buf) {
    struct task_struct *task;
    struct mm_struct *mm;
    unsigned long physmem = 0;

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task) {
        mm = task->mm;
        if (mm) {
            physmem = get_mm_rss(mm) << PAGE_SHIFT;
        }
    }
    rcu_read_unlock();

    switch (unit) {
    case 'K':
        physmem >>= 10;
        break;
    case 'M':
        physmem >>= 20;
        break;
    }

    return sprintf(buf, "%lu\n", physmem);
}

static struct kobj_attribute pid_attribute =
    __ATTR(pid, 0664, pid_show, pid_store);
static struct kobj_attribute virtmem_attribute =
    __ATTR(virtmem, 0444, virtmem_show, NULL);
static struct kobj_attribute physmem_attribute =
    __ATTR(physmem, 0444, physmem_show, NULL);
static struct kobj_attribute unit_attribute =
    __ATTR(unit, 0664, unit_show, unit_store);

static struct attribute *attrs[] = {
    &pid_attribute.attr,
    &virtmem_attribute.attr,
    &physmem_attribute.attr,
    &unit_attribute.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int __init mem_stats_init(void) {
    int ret;

    mem_stats_kobj = kobject_create_and_add("mem_stats", kernel_kobj);
    if (!mem_stats_kobj)
        return -1;

    ret = sysfs_create_group(mem_stats_kobj, &attr_group);
    if (ret)
        kobject_put(mem_stats_kobj);

    return ret;
}

static void __exit mem_stats_exit(void) { kobject_put(mem_stats_kobj); }

module_init(mem_stats_init);
module_exit(mem_stats_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("memory statistics");