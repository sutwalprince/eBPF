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

#define DEVICE_NAME "ioctl_mapper"
#define CLASS_NAME "ioctl"

#define IOCTL_MAGIC 'V'
#define IOCTL_GET_PHYSICAL _IOWR(IOCTL_MAGIC, 1, struct ioctl_translate)
#define IOCTL_WRITE_BYTE _IOW(IOCTL_MAGIC, 2, struct ioctl_write_data)

struct ioctl_translate
{
    unsigned long virt_addr;
    unsigned long phys_addr;
    int valid;
};

struct ioctl_write_data
{
    unsigned long phys_addr;
    unsigned char value;
};

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

static unsigned long virt_to_phys_user(unsigned long vaddr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page = NULL;
    unsigned long paddr = 0;
    unsigned long page_offset = vaddr & ~PAGE_MASK;

    struct mm_struct *mm = current->mm;

    if (!mm)
    {
        printk(KERN_ERR "Task2: No mm_struct \n");
        return 0;
    }

    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd))
    {
        return 0;
    }

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d))
    {
        return 0;
    }

    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud))
    {
        return 0;
    }

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd))
    {
        return 0;
    }

    pte = pte_offset_kernel(pmd, vaddr);
    if (!pte)
    {
        return 0;
    }

    if (!pte_present(*pte))
    {
        printk(KERN_ERR "Task2: Page not present\n");
        pte_unmap(pte);
        return 0;
    }

    page = pte_page(*pte);
    pte_unmap(pte);

    if (!page)
    {
        printk(KERN_ERR "Task2: Cannot get page from PTE\n");
        return 0;
    }

    paddr = (page_to_phys(page)) | page_offset;

    printk(KERN_INFO "Task2: Translated vaddr 0x%lx -> paddr 0x%lx\n",
           vaddr, paddr);

    return paddr;
}

static int write_phys_byte(unsigned long phys_addr, unsigned char value)
{
    void *virt_addr;
    struct page *page;
    unsigned long offset;
    unsigned long pfn = phys_addr >> PAGE_SHIFT;

    if (!pfn_valid(pfn))
    {
        printk(KERN_ERR "Task2: Invalid physical address 0x%lx\n", phys_addr);
        return -1;
    }

    page = pfn_to_page(pfn);
    if (!page)
    {
        printk(KERN_ERR "Task2: Cannot get page for pfn %lu\n", pfn);
        return -1;
    }

    virt_addr = kmap(page);
    if (!virt_addr)
    {
        printk(KERN_ERR "Task2: Cannot map page\n");
        return -1;
    }

    offset = phys_addr & ~PAGE_MASK;

    *((unsigned char *)virt_addr + offset) = value;

    kunmap(page);

    printk(KERN_INFO "Task2: Wrote 0x%02x to physical address 0x%lx\n",
           value, phys_addr);

    return 0;
}

static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Task2: Device opened by PID %d\n", current->pid);
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Task2: Device closed by PID %d\n", current->pid);
    return 0;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct ioctl_translate trans_data;
    struct ioctl_write_data write_data;
    unsigned long phys_addr;
    int ret;

    printk(KERN_INFO "Task2: ioctl called with cmd=0x%x by PID %d\n",
           cmd, current->pid);

    if (cmd == IOCTL_GET_PHYSICAL)

    {
        if (copy_from_user(&trans_data, (struct ioctl_translate __user *)arg,
                           sizeof(struct ioctl_translate)))
        {
            printk(KERN_ERR "Task2: Failed to copy from user\n");
            return -1;
        }

        printk(KERN_INFO "Task2: Translating virtual address 0x%lx\n",
               trans_data.virt_addr);

        phys_addr = virt_to_phys_user(trans_data.virt_addr);

        if (phys_addr == 0)
        {
            trans_data.valid = 0;
            trans_data.phys_addr = 0;
            printk(KERN_WARNING "Task2: Translation failed\n");
        }
        else
        {
            trans_data.valid = 1;
            trans_data.phys_addr = phys_addr;
        }

        if (copy_to_user((struct ioctl_translate __user *)arg, &trans_data,
                         sizeof(struct ioctl_translate)))
        {
            printk(KERN_ERR "Task2: Failed to copy to user\n");
            return -EFAULT;
        }
    }

    if (cmd == IOCTL_WRITE_BYTE)
    {
        if (copy_from_user(&write_data, (struct ioctl_write_data __user *)arg,
                           sizeof(struct ioctl_write_data)))
        {
            printk(KERN_ERR "Task2: Failed to copy from user\n");
            return -EFAULT;
        }

        printk(KERN_INFO "Task2: Writing 0x%02x to physical address 0x%lx\n",
               write_data.value, write_data.phys_addr);

        ret = write_phys_byte(write_data.phys_addr, write_data.value);
        if (ret < 0)
        {
            return ret;
        }
    }
    return 0;
}

static int __init ioctl_mapper_init(void)
{
    dev_t dev;
    int ret;

    printk(KERN_INFO "Task2: Initializing module\n");

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "Task2: Failed to allocate major number\n");
        return ret;
    }
    major_number = MAJOR(dev);
    printk(KERN_INFO "Task2: Registered with major number %d\n", major_number);

    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, dev, 1);
    if (ret < 0)
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Task2: Failed to add cdev\n");
        return ret;
    }

    device_class = class_create(CLASS_NAME);
    if (IS_ERR(device_class))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Task2: Failed to create class\n");
        return PTR_ERR(device_class);
    }

    device_driver = device_create(device_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(device_driver))
    {
        class_destroy(device_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Task2: Failed to create device\n");
        return PTR_ERR(device_driver);
    }

    printk(KERN_INFO "Task2: Device created successfully at /dev/%s\n",
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

    printk(KERN_INFO "Task2: Module removed\n");
}

module_init(ioctl_mapper_init);
module_exit(ioctl_mapper_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("Virtual to Physical address");
