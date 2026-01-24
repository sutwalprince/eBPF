#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/highmem.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("LKM4: Virtual vs Physical Memory Usage (Maple Tree)");

static int pid = -1;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid, "PID of the process");

static int __init lkm4_init(void)
{
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    unsigned long vma_size = 0;
    unsigned long phys_pages = 0;
    unsigned long addr;

    if (pid < 0)
    {
        printk(KERN_ERR "lkm4: Invalid PID\n");
        return -EINVAL;
    }


    task = pid_task(find_vpid(pid), PIDTYPE_PID);

    if (!task || !task->mm)
    {
        printk(KERN_ERR "lkm4: Process not found or no  memory\n");
        return -ESRCH;
    }

    mm = task->mm;


    VMA_ITERATOR(vmi, mm, 0);

    for_each_vma(vmi, vma)
    {
        vma_size += (vma->vm_end - vma->vm_start);
        for (addr = vma->vm_start; addr < vma->vm_end; addr += PAGE_SIZE)
        {

            pgd_t *pgd = pgd_offset(mm, addr);
            if (pgd_none(*pgd) )
                continue;

            p4d_t *p4d = p4d_offset(pgd, addr);
            if (p4d_none(*p4d) )
                continue;

            pud_t *pud = pud_offset(p4d, addr);
            if (pud_none(*pud) )
                continue;

            pmd_t *pmd = pmd_offset(pud, addr);
            if (pmd_none(*pmd) )
                continue;

            pte_t *pte = pte_offset_kernel(pmd, addr);
            if (!pte)
                continue;

            if (pte_present(*pte))
                phys_pages++;

        }
    }

    printk(KERN_INFO "lkm4: PID = %d\n", pid);
    printk(KERN_INFO "lkm4: Virtual Memory = %lu KB\n", vma_size / 1024);
    printk(KERN_INFO "lkm4: Physical Memory = %lu KB\n",
           (phys_pages * PAGE_SIZE) / 1024);
    printk(KERN_INFO "lkm4: Physical pages: %lu pagesize : %lu\n", phys_pages, PAGE_SIZE);

    return 0;
}

static void __exit lkm4_exit(void)
{
    printk(KERN_INFO "lkm4: Module unloaded\n");
}

module_init(lkm4_init);
module_exit(lkm4_exit);
