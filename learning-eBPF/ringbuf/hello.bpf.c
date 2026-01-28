#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include "hello.h"
#include <bpf/bpf_core_read.h>

/*
how to find params in syscalls

1.sudo bash
2.cd /sys/kernel/debug/tracing/events/syscalls/<syscall_name>
3.cat format


name: sys_enter_execve
ID: 843
format:
field:unsigned short common_type;       offset:0;       size:2; signed:0;
field:unsigned char common_flags;       offset:2;       size:1; signed:0;
        field:unsigned char common_preempt_count;       offset:3;       size:1; signed:0;
        field:int common_pid;   offset:4;       size:4; signed:1;

        field:int __syscall_nr; offset:8;       size:4; signed:1;
        field:const char * filename;    offset:16;      size:8; signed:0;
        field:const char *const * argv; offset:24;      size:8; signed:0;
        field:const char *const * envp; offset:32;      size:8; signed:0;


        REMEMBER TO CHECK OFFSET for arguments in your system (offset * 8 bits to get correct argument)
*/

struct execve_args {
    u64 unused1; //64 bits not need right now
    u64 unused2; //64 bits not need right now
    const char *filename; // offset 16 bytes so 16*8=128 bits not used 
};

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 20); //1MB
} ringbuf_map SEC(".maps");

SEC("tp/syscalls/sys_enter_execve")
int handle_execve(struct execve_args *params)
{
    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    struct hello_event *evt = {0};

    evt = bpf_ringbuf_reserve(&ringbuf_map, sizeof(*evt), 0);
    if (!evt) {
        return 0; // failed to reserve space
    }

    evt->pid = BPF_CORE_READ(task, pid);
    evt->tgid = BPF_CORE_READ(task, tgid);
    bpf_get_current_comm(&evt->comm, sizeof(evt->comm));
    bpf_probe_read_user_str(&evt->file, sizeof(evt->file), params->filename);
    bpf_ringbuf_submit(evt, 0);
    bpf_printk("Hello, eBPF! from %d\n", bpf_get_current_pid_tgid() >> 32);

    return 0;
}

char LICENSE[] SEC("license") = "GPL";  