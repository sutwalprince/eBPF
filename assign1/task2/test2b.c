#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/ioctl_mapper"

#define IOCTL_MAGIC 'V'
#define IOCTL_GET_PHYSICAL _IOWR(IOCTL_MAGIC, 1, struct ioctl_translate)
#define IOCTL_WRITE_BYTE _IOW(IOCTL_MAGIC, 2, struct ioctl_write_data)

struct ioctl_translate
{
    unsigned long virt_addr;
    unsigned long phys_addr;
    int valid;
};

struct ioctl_write_data
{
    unsigned long phys_addr;
    unsigned char value;
};




char *allocate_memory(size_t count)
{
    char *memory = (char *)malloc(count);
    if (!memory)
    {
        fprintf(stderr, "ERROR: Failed to allocate %zu bytes\n", count);
        return NULL;
    }
    return memory;
}


void initialize_memory(unsigned char *memory, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        memory[i] = 104 + i;
    }

}


void print_memory_contents(unsigned char *memory, size_t count)
{

    printf("%-6s %-18s %-10s\n", "Index", "Virtual Address", "Value");

    for (size_t i = 0; i < count; i++)
    {
        printf("%-6zu %p      %-10u \n",
               i, (void *)&memory[i], memory[i]);
    }

    printf("\n");
}


int get_physical_addresses(int fd, unsigned char *memory, size_t count,
                           unsigned long *phys_addrs)
{
    struct ioctl_translate trans;



    for (size_t i = 0; i < count; i++)
    {
        trans.virt_addr = (unsigned long)&memory[i];

        if (ioctl(fd, IOCTL_GET_PHYSICAL, &trans) < 0)
        {
            fprintf(stderr, "ERROR: ioctl failed for index %zu: %s\n",
                    i, strerror(errno));
            return -1;
        }

        if (!trans.valid)
        {
            fprintf(stderr, "ERROR: Translation failed for index %zu\n", i);
            return -1;
        }

        phys_addrs[i] = trans.phys_addr;

    }

    return 0;
}


int update_via_physical(int fd, unsigned long *phys_addrs, size_t count)
{
    struct ioctl_write_data write_data;


    for (size_t i = 0; i < count; i++)
    {
        write_data.phys_addr = phys_addrs[i];
        write_data.value = 53 + i;

        if (ioctl(fd, IOCTL_WRITE_BYTE, &write_data) < 0)
        {
            fprintf(stderr, "ERROR: Write failed for index %zu: %s\n",
                    i, strerror(errno));
            return -1;
        }

    }

    return 0;
}


int main(int argc, char *argv[])
{
    int fd;
    size_t count;
    unsigned char *memory = NULL;
    unsigned long *phys_addrs = NULL;
    int ret = EXIT_FAILURE;

    if (argc > 1)
    {
        count = atoi(argv[1]);
    }
    else
    {
        count = 10; 
    }

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device");
        return 0;
    }

    memory = allocate_memory(count);
    if (!memory)
    {
        free(memory);

        if (phys_addrs)
        {
            free(phys_addrs);
        }
        if (fd >= 0)
        {
            close(fd);
        }
        return 0;
    }

    phys_addrs = (unsigned long *)malloc(count * sizeof(unsigned long));
    if (!phys_addrs)
    {
        if (memory)
        {
            free(memory);
        }
        if (phys_addrs)
        {
            free(phys_addrs);
        }
        if (fd >= 0)
        {
            close(fd);
        }
        return 0;
    }

    initialize_memory(memory, count);
    print_memory_contents(memory, count );

    if (get_physical_addresses(fd, memory, count, phys_addrs) < 0)
    {
        if (memory)
        {
            free(memory);
        }
        if (phys_addrs)
        {
            free(phys_addrs);
        }
        if (fd >= 0)
        {
            close(fd);
        }
        return 0;
    }

    if (update_via_physical(fd, phys_addrs, count) < 0)
    {
        if (memory)
        {
            free(memory);
        }
        if (phys_addrs)
        {
            free(phys_addrs);
        }
        if (fd >= 0)
        {
            close(fd);
        }
        return 0;
    }

    

    print_memory_contents(memory, count);

    return 0;
}