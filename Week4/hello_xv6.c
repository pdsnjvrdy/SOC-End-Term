#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    printf("Hello, xv6! This is Sanjeeva\n");
    printf("Number of argments: %d\n", argc-1);

    for (int i = 1; i < argc; i++) {
        printf(" arg %d: %s\n", i, argv[i]);
    }

    exit(0);
}