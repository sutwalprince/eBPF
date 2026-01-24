
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>

MODULE_AUTHOR("Prince");
MODULE_DESCRIPTION("LKM3: Virtual to Physical Address Translation");
MODULE_LICENSE("GPL");

static int pid = 0;
module_param(pid, int, 0444);
MODULE_PARM_DESC(pid, "Process ID");

static unsigned long vaddr = 0;
module_param(vaddr, ulong, 0444);
MODULE_PARM_DESC(vaddr, "Virtual Address ");

static int __init custom_init(void)
{
    struct task_struct *task;
    struct mm_struct *mm;

    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    struct page *page;
    unsigned long paddr;

    printk(KERN_INFO "lkm3: Loading module\n");

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        printk(KERN_ERR "lkm3: Invalid PID %d\n", pid);
        return -1;
    }

    mm = task->mm;
    if (!mm) {
        printk(KERN_ERR "lkm3: PID %d has no user memory\n", pid);
        return -1;
    }

    
 
    pgd = pgd_offset(mm, vaddr);
    printk(KERN_INFO " PGD entry: 0x%lx\n", pgd_val(*pgd));
    if (pgd_none(*pgd) || pgd_bad(*pgd))
        goto unmapped;

    p4d = p4d_offset(pgd, vaddr);
    printk(KERN_INFO " P4D entry: 0x%lx\n", p4d_val(*p4d));
    if (p4d_none(*p4d) || p4d_bad(*p4d))
        goto unmapped;

    pud = pud_offset(p4d, vaddr);
    printk(KERN_INFO " PUD entry: 0x%lx\n", pud_val(*pud));
    if (pud_none(*pud) || pud_bad(*pud))
        goto unmapped;

    pmd = pmd_offset(pud, vaddr);
    printk(KERN_INFO " PMD entry: 0x%lx\n", pmd_val(*pmd));
    if (pmd_none(*pmd) || pmd_bad(*pmd))
        goto unmapped;

    pte = pte_offset_kernel(pmd, vaddr);
    printk(KERN_INFO " PTE entry: 0x%lx\n", pte_val(*pte));
    if (!pte || !pte_present(*pte))
        goto unmapped;

    page = pte_page(*pte);
    printk(KERN_INFO "lkm3: Page frame number: 0x%lx\n", page_to_pfn(page));
    paddr = page_to_phys(page) | (vaddr & ~PAGE_MASK);

    printk(KERN_INFO "PID: %d\n", pid);
    printk(KERN_INFO "Virtual Address: %lu\n", vaddr);
    printk(KERN_INFO "Physical Address: 0x%lx\n", paddr);

    return 0;

unmapped:
    mmap_read_unlock(mm);
    printk(KERN_INFO "lkm3: Virtual address %lu is NOT mapped for PID %d\n",
           vaddr, pid);
    return 0;
}

static void __exit custom_exit(void)
{
    printk(KERN_INFO "lkm3: Module unloaded\n");
}

module_init(custom_init);
module_exit(custom_exit);




// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/init.h>
// #include <linux/sched/signal.h>
// #include <linux/sched.h>
// #include <linux/mm.h>

// // Module metadata
// MODULE_AUTHOR("Prince");
// MODULE_DESCRIPTION("Peeking into Linux Kernel ");
// MODULE_LICENSE("GPL");

// static int pid = 0;
// module_param(pid, int, S_IRUGO);
// MODULE_PARM_DESC(pid, "Process ID");

// static unsigned long vaddr = 0;
// module_param(vaddr, ulong, S_IRUGO);
// MODULE_PARM_DESC(vaddr, "Virtual Address");

// static int __init custom_init(void)
// {
//     struct task_struct *task;
//     struct mm_struct *mm;

//     printk(KERN_INFO "\nLoading Module.");

//     struct page *page;
//     pgd_t *pgd;
//     p4d_t *p4d;
//     pmd_t *pmd;
//     pud_t *pud;
//     pte_t *pte;
//     void *laddr; // *paddr;
//     unsigned long paddr;

//     task = pid_task(find_vpid(pid), PIDTYPE_PID);
//     if (task == NULL)
//     {
//         printk(KERN_INFO "\nInvalid PID.");
//         return -1;
//     }

//     mm = task->mm;
//     if (!mm)
//     {
//         printk(KERN_INFO "\nNo memory map found for the given PID.");
//         return -1;
//     }

//     mmap_read_lock(mm);
//     pgd = pgd_offset(mm, vaddr);
//     p4d = p4d_offset(pgd, vaddr);
//     pud = pud_offset(p4d, vaddr);
//     pmd = pmd_offset(pud, vaddr);
//     pte = pte_offset_map(pmd, vaddr);
//     page = pte_page(*pte);
//     // laddr = page_address(page);
//     paddr = page_to_phys(page);
//     pte_unmap(pte);

//     mmap_read_unlock(mm);
//     printk(KERN_INFO "PID: %d\n", pid);
//     printk(KERN_INFO "Virtual Address: 0x%lx\n", vaddr);
//     printk(KERN_INFO "Physical Address: 0x%lx\n", paddr);

//     // for_each_process(task)
//     // {
//     //     if (task->parent->pid == pid)
//     //     {
//     //         printk(KERN_INFO "\nPID: %d | Name: %s | State: %ld\n",
//     //                task->pid, task->comm, task->__state);
//     //     }
//     // }
//     // printk(KERN_INFO "\nAll child processes of PID %d printed.", pid);
//     return 0;
// }
// static void __exit custom_exit(void)
// {
//     printk(KERN_INFO "\nUnloading Module.");
// }
// module_init(custom_init);
// module_exit(custom_exit);