#include "kernel/types.h"
#include "user/user.h"

// Explicit prototype to avoid implicit declaration error
int sleep(int);

int main(void) {
    int pid = getpid();
    printf("Syscall Spy Report for PID %d:\n", pid);

    // 1. getpid()
    printf("  getpid()       → %d\n", pid);

    // 2. uptime()
    printf("  uptime()       → %d\n", uptime());

    // 3. fork()
    int child_pid = fork();
    if (child_pid < 0) {
        printf("fork failed\n");
        exit(1);
    }

    if (child_pid == 0) {
        // Child process
        printf("--- in child (PID %d) ---\n", getpid());
        printf("  getpid()       → %d\n", getpid());
        exit(0);
    } else {
        // Parent process
        printf("  fork()         → %d  (child PID)\n", child_pid);
        printf("--- back in parent (PID %d) ---\n", getpid());

        // 4. sleep(10)
        int ret_sleep = sleep(10);
        printf("  sleep(10)      → %d\n", ret_sleep);

        // 5. write(1, buf, n)
        char *msg = "Hello, world!\n";
        int ret_write = write(1, msg, 14);
        printf("  write(1, ...)  → %d\n", ret_write);

        // 6. dup(0)
        int ret_dup = dup(0);
        printf("  dup(0)         → %d\n", ret_dup);

        // 7. wait() – reap the child
        int waited = wait(0);
        printf("  wait()         → %d  (reaped child PID %d)\n", waited, waited);

        // 8. getpid() again
        int pid2 = getpid();
        printf("  getpid() (again) → %d\n", pid2);

        exit(0);
    }

    return 0;
}