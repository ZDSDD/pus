#include <stdio.h>
#include <unistd.h>
main()
{
    int n, i;
    int p[2];
    pipe(p);
    if (fork() == 0)
    {
        close(p[1]); /* proces potomny czyta */
        for (;;)
        {
            read(p[0], &n, sizeof(int));
            for (i = 0; i < n; i++)
            {
                printf("%c", 64 + n);
                fflush(stdout);
                sleep(1);
            }
        }
    }
    else
    {
        close(p[0]); /* proces macierzysty pisze */
        for (;;)
        {
            printf("parent: ");
            fflush(stdout);
            scanf("%d", &n);
            write(p[1], &n, sizeof(int));
        }
    }
}