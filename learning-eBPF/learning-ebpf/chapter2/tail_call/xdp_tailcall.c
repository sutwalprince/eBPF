#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <net/if.h>

#include <bpf/libbpf.h>
#include "xdp_tailcall.skel.h"

static volatile sig_atomic_t exiting = 0;

static void sig_handler(int sig)
{
    exiting = 1;
}

int main(int argc, char **argv)
{
    struct xdp_tailcall_bpf *skel = NULL;
    struct bpf_link *link = NULL;
    int ifindex;
    int err = 0;

    ifindex = if_nametoindex("ens3"); 
    if (!ifindex) {
        perror("if_nametoindex");
        return 1;
    }

    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

    skel = xdp_tailcall_bpf__open();
   

    xdp_tailcall_bpf__load(skel);

    link = bpf_program__attach_xdp(skel->progs.xdp_prog, ifindex);
    if (!link) {
        fprintf(stderr, "Failed to attach XDP program\n");
        err = 1;
        goto cleanup;
    }

    {
        __u32 k1 = 1, k5 = 5;
        int tail1_fd = bpf_program__fd(skel->progs.xdp_tail_1);
        int tail5_fd = bpf_program__fd(skel->progs.xdp_tail_5);

        bpf_map_update_elem(
            bpf_map__fd(skel->maps.jmp_table1),
            &k1,
            &tail1_fd,
            BPF_ANY);

        bpf_map_update_elem(
            bpf_map__fd(skel->maps.jmp_table1),
            &k5,
            &tail5_fd,
            BPF_ANY);
    }

    printf("Press Ctrl+C to exit\n");

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    while (!exiting)
        sleep(1);

cleanup:
    if (link)
        bpf_link__destroy(link);
    if (skel)
        xdp_tailcall_bpf__destroy(skel);

    return err;
}
