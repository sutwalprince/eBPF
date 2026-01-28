#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

SEC("tp/syscalls/sys_enter_execve")
int handle_execve(void *ctx)
{
    
    bpf_printk("Hello, eBPF! from %d\n" ,bpf_get_current_pid_tgid() >> 32 );
    
    return 0;
}

char LICENSE[] SEC("license") = "GPL";