#include <stdio.h>
#include <unistd.h>

void loop(char c)
{
    for (;;)
    {
        printf("%c", c);
        fflush(stdout);
        sleep(3);
    }
}

main()
{
    if (fork() > 0)
        loop('*');
    else
        loop('#');
}