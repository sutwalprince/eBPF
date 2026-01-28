#ifndef __HELLO_H_
#define __HELLO_H_

struct hello_event {
    pid_t pid;
    pid_t tgid ;
    char comm[32];
    char file[32];
};
#endif // __HELLO_H_ 