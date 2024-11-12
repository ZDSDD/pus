#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void handler_sigcld(int sig)
{
    int status, pid;
    // while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    // {
    //     printf("[zako´nczono proces o pid=%d]\n", pid);
    //     fflush(stdout);
    // }
    wait(pid);
    printf("[zako´nczono proces o pid=%d]\n", pid);
    fflush(stdout);
}

void childloop(int n)
{
    int i;
    printf("podano: %d\n", n);
    for (i = 0; i < n; i++)
    {
        printf("%c\n", 64 + n);
        fflush(stdout);
        sleep(1);
    }
    exit(0);
}

int main()
{
    int n;
    signal(SIGCLD, handler_sigcld);
    for (;;)
    {
        scanf("%d", &n);
        if (n <= 0)
            break;
        if (fork() == 0)
            childloop(n);
    }
    return 0;
}