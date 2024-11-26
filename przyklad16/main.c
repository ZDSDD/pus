#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#define TSIZE 10
void inc(int T[], int off)
{
    int n;
    for (;;)
    {
        for (n = off; n < TSIZE; n += 2)
            T[n]++;
        sleep(1);
    }
}
main(int argc, char *argv[])
{
    int d, n;
    int *T;
    printf("%s\n", argv[0]);
    d = shmget(ftok(argv[0], 1),
               TSIZE * sizeof(int), IPC_CREAT | 0600);
    T = shmat(d, NULL, 0);
    if (fork() == 0)
        inc(T, 0);
    if (fork() == 0)
        inc(T, 1);
    for (;;)
    {
        for (n = 0; n < TSIZE; n++)
            printf("%3d ", T[n]);
        printf("\n");
        sleep(1);
    }
}