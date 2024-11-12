#include <stdio.h>
#include <unistd.h>
main(int argc, char *argv[], char *envp[])
{
    char *a[] = {"/bin/ls", "-l", NULL};
    printf("Start\n");
    execve(a[0], a, envp);
    printf("Stop\n");
    return 0;
}