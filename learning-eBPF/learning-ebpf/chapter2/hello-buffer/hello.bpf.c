#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include "hello.h"


struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(int));
    __uint(value_size, sizeof(u32));
    __uint(max_entries, 2);
} my_map SEC(".maps");



// Use the generic syscall__ prefix for portability
SEC("kprobe/__x64_sys_execve")
int kprobe_execve_non_core(void *ctx)
{
    struct data_t data = {};
    char message[12] = "Hello World";

    data.pid = bpf_get_current_pid_tgid() >> 32;
    data.uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;

    bpf_get_current_comm(&data.command, sizeof(data.command));
    bpf_probe_read_kernel(&data.message, sizeof(data.message), message);

    bpf_perf_event_output(ctx, &my_map, BPF_F_CURRENT_CPU, &data, sizeof(data));
    bpf_printk(" PID: %d, UID: %d, COMM: %s, MESSAGE: %s\n",
           data.pid, data.uid, data.command, data.message);

    return 0;
}

char LICENSE[] SEC("license") = "GPL";
