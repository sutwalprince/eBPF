#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <num_pages> <stride_bytes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    long num_pages = atol(argv[1]);
    long stride = atol(argv[2]);

    long page_size = sysconf(_SC_PAGESIZE);
    size_t total_size = num_pages * page_size;

    // Allocate memory (page-aligned)
    char *buf = aligned_alloc(page_size, total_size);
    if (!buf) {
        perror("aligned_alloc");
        exit(EXIT_FAILURE);
    }

    printf("PID: %d\n", getpid());
    printf("Allocated %ld pages (%zu bytes)\n", num_pages, total_size);
    printf("Stride: %ld bytes\n", stride);

    // Touch memory using stride
    for (size_t offset = 0; offset < total_size; offset += stride) {
        buf[offset] = 'A';   // write triggers page fault
    }

    // Keep process alive for observation
    printf("Press Enter to exit...\n");
    getchar();

    free(buf);
    return 0;
}
