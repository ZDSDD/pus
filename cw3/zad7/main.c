#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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
        l[strcspn(l, "\n")] = '\0'; // Remove the newline character from input

        if (strlen(l) == 0)
            continue; // If the line is empty, skip to the next loop

        system(l); // Execute the entire command line with arguments
    }
    return 0;
}