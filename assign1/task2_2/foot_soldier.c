#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define IOCTL_MAGIC 'P'
#define IOCTL_CHANGE_PARENT _IO(IOCTL_MAGIC, 1)

int open_driver(const char *driver_name)
{

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1)
    {
        perror("ERROR: could not open driver");
    }

    return fd_driver;
}

void close_driver(const char *driver_name, int fd_driver)
{

    int result = close(fd_driver);
    if (result == -1)
    {
        perror("ERROR: could not close driver");
    }
}

int main(int argc, char **argv)
{

    pid_t child_pid = getpid();
    // Open ioctl driver
    int fd_driver = open_driver("/dev/ioctl_earth");
    if (fd_driver == -1)
    {
        perror("ERROR: could not open driver");
        exit(EXIT_FAILURE);
    }

    printf("[CHILD:%d] changing Old parent:%d\n", child_pid, getppid());
    if (ioctl(fd_driver, IOCTL_CHANGE_PARENT) == -1)
    {
        perror("ERROR: ioctl failed");
        close_driver("/dev/ioctl_earth", fd_driver);
        exit(EXIT_FAILURE);
    }

    close_driver("/dev/ioctl_earth", fd_driver);

    sleep(10); 
    printf("child %d exiting\n", child_pid);
    printf("new parent:%d\n",  getppid());
    return 0;
}