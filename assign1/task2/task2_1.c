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



int test_translation(int fd)
{
    struct ioctl_translate trans;
    int var = 42;

    printf("Virtual to Physical Address Translation\n");

    printf("Virtual address of var: %p\n", (void *)&var);
    printf("Value: %d\n", var);

    trans.virt_addr = (unsigned long)&var;
    if (ioctl(fd, IOCTL_GET_PHYSICAL, &trans) < 0)
    {
        perror("ioctl IOCTL_GET_PHYSICAL failed");
        return -1;
    }

    if (trans.valid)
    {
        printf("Physical address: 0x%lx\n", trans.phys_addr);
    }
    else
    {
        printf("Translation failed (page not mapped)\n");
        return -1;
    }

    return 0;
}

int test_physical_write(int fd)
{
    struct ioctl_translate trans;
    struct ioctl_write_data write_data;
    unsigned int test_byte = 100;
    unsigned int *ptr;

    printf("TEST 2: Physical Memory Write\n");

    printf("Initial value: %d\n", test_byte);
    printf("Virtual address: %p\n", (void *)&test_byte);

    trans.virt_addr = (unsigned long)&test_byte;
    if (ioctl(fd, IOCTL_GET_PHYSICAL, &trans) < 0)
    {
        perror("ioctl IOCTL_GET_PHYSICAL failed");
        return -1;
    }

    if (!trans.valid)
    {
        printf("Translation failed\n");
        return -1;
    }

    printf("Physical address: 0x%lx\n", trans.phys_addr);

    printf("\n2. Writing 99 to physical address 0x%lx:\n", trans.phys_addr);
    write_data.phys_addr = trans.phys_addr;
    write_data.value = 99;

    if (ioctl(fd, IOCTL_WRITE_BYTE, &write_data) < 0)
    {
        perror("ioctl IOCTL_WRITE_BYTE failed");
        return -1;
    }

    ptr = &test_byte;
    printf("Current value at virtual address: %d\n", *ptr);

    return 0;
}



int main(void)
{
    int fd;
    int ret = 0;

    printf("\nOpening device %s\n", DEVICE_PATH);
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    if (test_translation(fd) < 0)
    {
        ret = -1;
    }

    if (test_physical_write(fd) < 0)
    {
        ret = -1;
    }

    

    return 0;
}