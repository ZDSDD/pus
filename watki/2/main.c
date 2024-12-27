#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
void *action(void *arg)
{
    sleep(*((int *)arg));
    return NULL;
}
main()
{
    pthread_t t1, t2, t3;
    int start;
    int tt[] = {3, 5, 8};
    void *res;
    pthread_create(&t1, NULL, action, tt);
    pthread_create(&t2, NULL, action, tt + 1);
    pthread_create(&t3, NULL, action, tt + 2);
    start = time(NULL);
    pthread_join(t3, &res);
    printf("%d\n", time(NULL) - start);
    pthread_join(t2, &res);
    printf("%d\n", time(NULL) - start);
    pthread_join(t1, &res);
    printf("%d\n", time(NULL) - start);
    return 0;
}