// SPDX-License-Identifier: GPL-2.0
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

char LICENSE[] SEC("license") = "GPL";


struct {
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(max_entries, 100);
    __type(key, __u32);
    __type(value, __u32);
} jmp_table1 SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_PROG_ARRAY);
    __uint(max_entries, 100);
    __type(key, __u32);
    __type(value, __u32);
} jmp_table2 SEC(".maps");


SEC("xdp")
int xdp_prog(struct xdp_md *ctx)
{
    bpf_printk("Root\n");

    bpf_tail_call(ctx, &jmp_table1, 1);

    bpf_printk("no tail call\n");
    return XDP_PASS;
}


SEC("xdp")
int xdp_tail_1(struct xdp_md *ctx)
{
    bpf_printk("Tail call 1\n");

    bpf_tail_call(ctx, &jmp_table1, 5);

    return XDP_PASS;
}


SEC("xdp")
int xdp_tail_5(struct xdp_md *ctx)
{
    bpf_printk("Tail call 2\n");
    bpf_printk("Packet size %d\n", ctx->data_end - ctx->data);
    return XDP_PASS;
}
