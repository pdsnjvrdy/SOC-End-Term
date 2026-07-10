#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int N = 5; 
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N < 1) N = 1;
    }

    int depth = 0;
    int pid;

    while (1) {
        printf("Depth %d: PID %d, Parent PID %d\n", depth, getpid(), getpid());

        if (depth == N - 1) {
            printf("I am the leaf! No more children.\n");
            // If N == 1, this process is also the original; print summary here
            if (depth == 0) {
                printf("Total processes in chain: %d, Depth reached: %d\n", N, N - 1);
            }
            exit(0);
        }
        pid = fork();
        if (pid < 0) {
            printf("fork failed\n");
            exit(1);
        }
        if (pid == 0) {
            depth++;
            continue;
        } else {
            wait(0);
            if (depth == 0) {
                printf("Total processes in chain: %d, Depth reached: %d\n", N, N - 1);
            }
            exit(0);
        }
    }

    return 0;
}