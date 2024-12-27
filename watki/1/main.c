#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
void *gen(void *arg)
{
    for (;;)
    {
        printf("%c", *((char *)arg));
        fflush(stdout);
        sleep(1);
    }
}
main()
{
    pthread_t t1, t2, t3;
    char s[] = ".ABC";
    pthread_create(&t1, NULL, gen, s + 1);
    pthread_create(&t2, NULL, gen, s + 2);
    pthread_create(&t3, NULL, gen, s + 3);
    gen(s + 0);
}