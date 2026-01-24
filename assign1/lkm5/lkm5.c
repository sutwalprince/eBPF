#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("LKM5: THP Huge Page Analysis (Linux 6.14)");

static int pid = -1;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid, "PID of the process");

static int __init lkm5_init(void)
{
    struct task_struct *task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;

    unsigned long vma_size = 0;
    unsigned long huge_pages = 0;
    unsigned long huge_vaddr = 0;
    unsigned long addr;

    if (pid < 0) {
        printk(KERN_ERR "lkm5: Invalid PID\n");
        return -EINVAL;
    }

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    rcu_read_unlock();

    if (!task || !task->mm) {
        printk(KERN_ERR "lkm5: Process not found or no memory\n");
        return -1;
    }

    mm = task->mm;

    VMA_ITERATOR(vmi, mm, 0);
    for_each_vma(vmi, vma) {
        vma_size += (vma->vm_end - vma->vm_start);
    }

    VMA_ITERATOR(vmi2, mm, 0);
    for_each_vma(vmi2, vma) {

        for (addr = vma->vm_start; addr < vma->vm_end; addr += HPAGE_PMD_SIZE) {

            pgd_t *pgd = pgd_offset(mm, addr);
            if (pgd_none(*pgd) || pgd_bad(*pgd))
                continue;

            p4d_t *p4d = p4d_offset(pgd, addr);
            if (p4d_none(*p4d) || p4d_bad(*p4d))
                continue;

            pud_t *pud = pud_offset(p4d, addr);
            if (pud_none(*pud) || pud_bad(*pud))
                continue;

            pmd_t *pmd = pmd_offset(pud, addr);
            if (!pmd)
                continue;

            if (pmd_present(*pmd) && pmd_trans_huge(*pmd)) {
                huge_pages++;
                huge_vaddr += HPAGE_PMD_SIZE;
            }
        }
    }

    printk(KERN_INFO "lkm5: PID = %d\n", pid);
    printk(KERN_INFO "lkm5: Virtual Memory = %lu KB\n", vma_size / 1024);
    printk(KERN_INFO "lkm5: Huge Pages = %lu\n", huge_pages);
    printk(KERN_INFO "lkm5: Huge Page Virtual Space = %lu KB\n",
           huge_vaddr / 1024);

    return 0;
}

static void __exit lkm5_exit(void)
{
    printk(KERN_INFO "lkm5: Module unloaded\n");
}

module_init(lkm5_init);
module_exit(lkm5_exit);
