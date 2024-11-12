#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
main()
{
    int status;
    if (fork() == 0)
    {
        int n;
        for (n = 0; n < 10; n++)
        {
            printf("*");
            fflush(stdout);
            sleep(1);
        }
        exit(7);
    }
    wait(10000);
    printf("\nKod powrotu %x\n", WEXITSTATUS(status));
    return 0;
}