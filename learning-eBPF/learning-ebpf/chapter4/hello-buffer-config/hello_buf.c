#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "hello_buf_config.skel.h"
#include <bpf/bpf.h>

static volatile sig_atomic_t exiting = 0;

struct user_msg_t
{
    char message[13];
};

struct data_t
{
    int pid;
    int uid;
    char command[16];
    char message[12];
};

static void handle_event(void *ctx, int cpu, void *data, __u32 size)
{
    struct data_t *e = data;
    if(e->uid != 0)
        return;
    printf("%d %d %s %s\n",
           e->pid, e->uid, e->command, e->message);
}

static void handle_lost(void *ctx, int cpu, __u64 lost)
{
    fprintf(stderr, "Lost %llu events on CPU %d\n", lost, cpu);
}

static void sig_handler(int sig)
{
    exiting = 1;
}

int main(void)
{

    struct perf_buffer *pb = NULL;
    struct user_msg_t msg;
    int key;
    int err;
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    struct hello_buf_config *skel = hello_buf_config__open();
    hello_buf_config__load(skel);
    if (!skel)
    {
        fprintf(stderr, "Failed to open/load BPF\n");
        return 1;
    }

    err = hello_buf_config__attach(skel);
    if (err)
    {
        fprintf(stderr, "Failed to attach BPF\n");
        goto cleanup;
    }

    key = 0;
    snprintf(msg.message, sizeof(msg.message), "Hey root!");
    bpf_map_update_elem(
        bpf_map__fd(skel->maps.my_map),
        &key, &msg, BPF_ANY);

    key = 501;
    snprintf(msg.message, sizeof(msg.message), "Hi user 501!");
    bpf_map_update_elem(
        bpf_map__fd(skel->maps.my_map),
        &key, &msg, BPF_ANY);

    pb = perf_buffer__new(bpf_map__fd(skel->maps.output), 8, handle_event, handle_lost, NULL, NULL);

    if (!pb)
    {
        fprintf(stderr, "Failed to create perf buffer\n");
        goto cleanup;
    }

    while (!exiting)
    {
        perf_buffer__poll(pb, 100);
    }

cleanup:
    perf_buffer__free(pb);
    hello_buf_config__destroy(skel);
    return 0;
}
