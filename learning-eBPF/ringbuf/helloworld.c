#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include "hello.skel.h"
#include "hello.h"

static void bump_memlock_rlimit(void)
{
    struct rlimit rlim_new = {
        .rlim_cur = RLIM_INFINITY,
        .rlim_max = RLIM_INFINITY,
    };
    if (setrlimit(RLIMIT_MEMLOCK, &rlim_new))
    {
        fprintf(stderr, "Failed to increase RLIMIT_MEMLOCK limit!\n");
        exit(1);
    }
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
    struct hello_event *evt = data;
    
    printf("PID: %d, TGID: %d, COMM: %s, FILENAME: %s\n",
           evt->pid, evt->tgid, evt->comm, evt->file);
    return 0;
}

int main()
{
    bump_memlock_rlimit();

    struct hello *skel = hello__open();
    hello__load(skel);
    hello__attach(skel);

    struct ring_buffer *rb = ring_buffer__new(bpf_map__fd(skel->maps.ringbuf_map), handle_event, NULL, NULL);

    while (1)
    {
        ring_buffer__poll(rb, 5000 /* timeout, ms */);
    }
    hello__destroy(skel);
    return 0;
}