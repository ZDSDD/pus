#include <stdio.h>
#include <string.h>
#include <unistd.h>
main(int argc, char *argv[], char *envp[])
{
    char *a[64]; /* maksymalna liczba
    argumentów polecenia to 62 */
    char l[256]; /* maksymalna długo´s´c
    linii polece´n to 255 */
    int n;
    for (;;)
    {
        printf("co wykonac? ");
        fflush(stdout);
        if (fgets(l, 256, stdin) == NULL)
            break;
        a[0] = strtok(l, " \t\n\r"); /* pobranie pierwszego
        wyrazu linii */
        if (a[0] == NULL)
            continue;
        n = 1;
        while (a[n] = strtok(NULL, " \t\n\r"))
            n++; /* pobranie kolejnych wyrazów */
        if (fork() == 0)
        {
            execvp(a[0], a);
            perror("execvp"); /* tylko w przypadku bł˛edu */
            exit(0);
        }
        else
            wait(&n);
        }
    return 0;
}