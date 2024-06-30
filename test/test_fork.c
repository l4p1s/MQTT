#include <unistd.h>
#include <stdio.h>

int main()
{
    int i = 0;
    pid_t pid;

    printf("fork start\n");
    if ((pid = fork()) == -1) {
        perror("fork");
        return 1;
    }
    else if (pid > 0) {
        printf("parent: waiting ...\n");
        wait(NULL);
        printf("parent: my child just finished\n");
    }
    else {
        for (i = 0; i < 4; i++) {
            printf("%02d: child running...\n", i);
            sleep(1);
        }
    }
    return 0;
}
