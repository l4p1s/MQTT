#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

struct h {
        int a;
        int b;
        int c;
        int d;
} hoge;

int main()
{
        hoge.a = 1;
        pid_t pid;
        pid = fork();
        if (pid) {
                // parrent process
                hoge.b = 2;
                if (pid != -1) {
                        int stat;
                        wait(&stat);
                } else {
                        printf("fork failed\n");
                }
                printf("p: %d %d %d %d\n", hoge.a, hoge.b, hoge.c, hoge.d);
        } else {
                // child process
                hoge.c = 3;
                printf("c: %d %d %d %d\n", hoge.a, hoge.b, hoge.c, hoge.d);
        }
        return 0;
}
