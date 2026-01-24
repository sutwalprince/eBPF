#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("Hello Sysfs Example");

static struct kobject *hello_kobj;

static int hello_int = 42;
static char hello_string[100] = "Hello from kernel by prince";

static ssize_t hello_int_show(struct kobject *kobj,
                              struct kobj_attribute *attr,
                              char *buf)
{
    return sprintf(buf, "%d\n", hello_int);
}

static ssize_t hello_int_store(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               const char *buf,
                               size_t count)
{
    sscanf(buf, "%d", &hello_int);
    return count;
}

static struct kobj_attribute hello_int_attr =
    __ATTR(hello_int, 0664, hello_int_show, hello_int_store);

static ssize_t hello_string_show(struct kobject *kobj,
                                 struct kobj_attribute *attr,
                                 char *buf)
{
    return sprintf(buf, "%s\n", hello_string);
}

static ssize_t hello_string_store(struct kobject *kobj,
                                  struct kobj_attribute *attr,
                                  const char *buf,
                                  size_t count)
{
    snprintf(hello_string, sizeof(hello_string), "%.*s",
             (int)(count - 1), buf);
    return count;
}

static struct kobj_attribute hello_string_attr =
    __ATTR(hello_string, 0664, hello_string_show, hello_string_store);

static int __init hello_sysfs_init(void)
{
    int ret;

    hello_kobj = kobject_create_and_add("hello_sysfs", kernel_kobj);
    if (!hello_kobj)
        return -1;

    ret = sysfs_create_file(hello_kobj, &hello_int_attr.attr);
    if (ret)
        goto err;

    ret = sysfs_create_file(hello_kobj, &hello_string_attr.attr);
    if (ret)
        goto err;

    printk(KERN_INFO "hello_sysfs module loaded\n");
    return 0;

err:
    kobject_put(hello_kobj);
    return ret;
}

static void __exit hello_sysfs_exit(void)
{
    kobject_put(hello_kobj);
    printk(KERN_INFO "hello_sysfs module unloaded\n");
}

module_init(hello_sysfs_init);
module_exit(hello_sysfs_exit);
