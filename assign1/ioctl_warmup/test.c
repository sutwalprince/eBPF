#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/simple_ioctl"
#define IOCTL_MAGIC 'S'
#define IOCTL_GET_CONSTANT _IOR(IOCTL_MAGIC, 1, int)

int main() {
    int fd;
    int value;
    int ret;

    printf("Opening device %s...\n", DEVICE_PATH);
    
    /* Open the device file */
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf("Device opened successfully (fd=%d)\n", fd);

    /* Call ioctl to get the constant */
    printf("Calling ioctl\n");
    ret = ioctl(fd, IOCTL_GET_CONSTANT, &value);
    
    if (ret < 0) {
        perror("ioctl failed");
        close(fd);
        return -1;
    }

    printf("SUCCESS! ioctl returned: %d\n", value);
    close(fd);
    printf("Device closed\n");

    return EXIT_SUCCESS;
}