#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define IOCTL_MAGIC 'P'
#define IOCTL_TERMINATE_CHILDREN _IOW(IOCTL_MAGIC, 2, pid_t)

void sigchld_handler(int sig) {
    int status;
    printf("[Parent]: Received SIGCHLD\n");
    fflush(stdout);
    pid_t soldier_pid;
    while ((soldier_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[Parent]: Soldier process %d terminated\n", soldier_pid);
        fflush(stdout);
    }
    if (soldier_pid == -1 && errno != ECHILD) {
        perror("waitpid");
    }
}

void sigterm_handler(int sig) {
    printf("[Parent]: Control station %d exiting\n", getpid());
    fflush(stdout);

    exit(0);
}

int open_driver(const char *driver_name) {

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
    }

    return fd_driver;
}

void close_driver(const char *driver_name, int fd_driver) {

    int result = close(fd_driver);
    if (result == -1) {
        perror("ERROR: could not close driver");
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage: %s <sleep>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int s_d = atoi(argv[1]);

    signal(SIGCHLD, sigchld_handler);
    signal(SIGTERM, sigterm_handler);

    printf("[PARENT:%u] Control station process started\n", getpid());
    sleep(s_d); // Wait for all child process until Emergency is initiated.

    printf("[PARENT]: Emergency Emergency!\n");

    // open ioctl driver
    int fd_driver = open_driver("/dev/ioctl_earth");
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = getpid();
    if (ioctl(fd_driver, IOCTL_TERMINATE_CHILDREN, &parent_pid) == -1) {
        perror("ERROR: ioctl failed");
        close_driver("/dev/ioctl_earth", fd_driver);
        exit(EXIT_FAILURE);
    }

    close_driver("/dev/ioctl_earth", fd_driver);

    printf("[PARENT]: Control station %u exiting", getpid());

    return 0;
}