#include <stdio.h>
#include <unistd.h>

int var = 123;

int main() {
    printf("PID: %d\n", getpid());
    printf("Address of var: %p\n", &var);
    getchar();   
    return 0;
}
