#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
int x = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *gen(void *arg)
{
    int n;
    for (;;)
    {
        while (x == 0)
            ;
        n = x;
        sleep(n);
        pthread_mutex_lock(&mutex);
        while (n-- > 0)
        {
            printf("%c", *((char *)arg));
            fflush(stdout);
            usleep(100000);
        }
        printf("\n");
        pthread_mutex_unlock(&mutex);
    }
}
main()
{
    pthread_t t1, t2;
    char s[] = "AB";
    int r;
    srand(time(NULL));
    pthread_create(&t1, NULL, gen, s);
    pthread_create(&t2, NULL, gen, s + 1);
    for (;;)
    {
        r = 5 + rand() % 5;
        printf("--- %d ---\n", r);
        x = r;
        sleep(1);
        x = 0;
        sleep(r);
    }
}