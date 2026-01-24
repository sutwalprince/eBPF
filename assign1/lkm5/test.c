#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <size_in_MB>\n", argv[0]);
        return 1;
    }

    size_t size_mb = atol(argv[1]);
    size_t size = size_mb * 1024 * 1024;

    printf("PID: %d\n", getpid());
    printf("Allocating %zu MB anonymous memory\n", size_mb);

    char *buf = mmap(NULL, size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);

    if (buf == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    
    for (size_t i = 0; i < size; i += 4096) {
        buf[i] = 1;
    }

    printf("Memory touched. Press Enter to exit...\n");
    getchar();

    munmap(buf, size);
    return 0;
}
