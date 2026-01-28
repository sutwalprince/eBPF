#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

#define TASK_COMM_LEN 16

struct user_msg_t {
    char message[13];
};

struct data_t {
    u32 pid;
    u32 uid;
    char command[TASK_COMM_LEN];
    char message[12];
};

/* HASH map: UID -> message */
struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 1024);
    __type(key, u32);
    __type(value, struct user_msg_t);
} my_map SEC(".maps");

/* PERF OUTPUT map */
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(u32));
    __uint(value_size, sizeof(u32));
    __uint(max_entries, 128);
} output SEC(".maps");

SEC("kprobe/__x64_sys_execve")
int hello(struct pt_regs *ctx)
{
    struct data_t data = {};
    struct user_msg_t *p;
    char default_msg[12] = "Hello World";

    data.pid = bpf_get_current_pid_tgid() >> 32;
    data.uid = bpf_get_current_uid_gid();

    bpf_get_current_comm(&data.command, sizeof(data.command));

    p = bpf_map_lookup_elem(&my_map, &data.uid);
    if (p) {
        __builtin_memcpy(data.message, p->message,
                          sizeof(data.message));
    } else {
        __builtin_memcpy(data.message, default_msg,
                          sizeof(default_msg));
    }

    bpf_perf_event_output(ctx, &output,
                          BPF_F_CURRENT_CPU,
                          &data, sizeof(data));
    return 0;
}

char LICENSE[] SEC("license") = "GPL";
