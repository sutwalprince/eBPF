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

static int handle_event(void *ctx,int cpu , void *data,  __u32 data_sz)
{
    struct data_t *evt = data;
    printf("PID: %d, TGID: %d, COMM: %s, MESSAGE: %s\n",
           evt->pid, evt->uid, evt->command, evt->message);
    return 0;
}
static int handle_lost(void *ctx,int cpu ,  __u64 lost_cnt)
{   
    return 0;
}

int main()
{
    bump_memlock_rlimit();

    struct hello *skel = hello__open();
    hello__load(skel);
    hello__attach(skel);

    struct perf_buffer *rb = perf_buffer__new(bpf_map__fd(skel->maps.my_map), 8, (void *)handle_event, (void *)handle_lost, NULL , NULL); ;

    while (1)
    {
        perf_buffer__poll(rb, 5000 /* timeout, ms */);
    }
    hello__destroy(skel);
    return 0;
}