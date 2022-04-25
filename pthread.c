#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
pthread_mutex_t mutex;
int wg = 0;
void *fun(void *arg)
{
    for (int i = 0; i < 10; i++)
    {
        pthread_mutex_lock(&mutex);
        wg++;
        printf("wg=%d\n", wg);
        pthread_mutex_unlock(&mutex);
    }
}
int main()
{
    pthread_t id[10];
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&id[i], NULL, fun, NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(id[i], NULL);
    }
}