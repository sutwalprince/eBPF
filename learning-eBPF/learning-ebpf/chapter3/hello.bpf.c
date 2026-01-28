#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

int counter = 0;

SEC("xdp")
int hello(struct xdp_md *ctx) {
    bpf_printk("Hello World %d", counter);
    __sync_fetch_and_add(&counter, 1);
    return XDP_PASS;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";
