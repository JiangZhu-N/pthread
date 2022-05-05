#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
//缓冲区
#define MIX 5

struct MPMCQueue
{
    /* Define Your Data Here */
    struct MPMCQueue *next;
    void *Data1;
    void *Data2;
} typedef MPMCQueue;

MPMCQueue *head1;
MPMCQueue *head2;

pthread_mutex_t lock;
pthread_cond_t notEmpty1;
pthread_cond_t notEmpty2;
pthread_cond_t notFull;

int bufferLeft = MIX;
int data1 = 0;
int data2 = 0;

MPMCQueue *MPMCQueueInit()
{
    MPMCQueue *head;
    head = malloc(sizeof(MPMCQueue));
    return head;
}

void MPMCQueueDestory(MPMCQueue *pool)
{
    free(pool);
}

void MPMCQueuePush1(MPMCQueue *pool, void *s)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        if (bufferLeft > 0)
        {

            pool = MPMCQueueInit();
            pool->Data1 = s;
            pool->next = head1;
            head1 = pool;
            bufferLeft--;
            data1++;
            printf("Push   1    [1]:%d    [2]:%d    总数:%d\n", data1, data2, MIX - bufferLeft);
            pthread_cond_signal(&notEmpty1);
        }
        else
        {
            printf("Push   1    失败:缓冲区已满\n");
            pthread_cond_wait(&notFull, &lock);
        }
        pthread_mutex_unlock(&lock);
        sleep(rand() % 2);
    }
}

void MPMCQueuePush2(MPMCQueue *pool, void *s)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        if (bufferLeft > 0)
        {

            pool = MPMCQueueInit();
            pool->Data2 = s;
            pool->next = head1;
            head1 = pool;
            bufferLeft--;
            data2++;
            printf("Push   2    [1]:%d    [2]:%d    总数:%d\n", data1, data2, MIX - bufferLeft);
            pthread_cond_signal(&notEmpty2);
        }
        else
        {
            printf("Push   2    失败:缓冲区已满\n");
            pthread_cond_wait(&notFull, &lock);
        }
        pthread_mutex_unlock(&lock);
        sleep(rand() % 2);
    }
}

void *MPMCQueuePop1(MPMCQueue *pool)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        while (data1 <= 0)
        {
            if (data1 <= 0)
            {
                printf("Pop    1    失败:无1号数据\n");
            }
            pthread_cond_wait(&notEmpty1, &lock);
        }
        pool = head1;
        head1 = pool->next;
        bufferLeft++;
        data1--;
        printf("Pop    1    [1]:%d    [2]:%d    总数:%d\n", data1, data2, MIX - bufferLeft);
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&lock);
        MPMCQueueDestory(pool);
        sleep(rand() % 2);
    }
}

void *MPMCQueuePop2(MPMCQueue *pool)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        while (data2 <= 0)
        {
            if (data2 <= 0)
            {
                printf("Pop    2    失败:无2号数据\n");
            }
            pthread_cond_wait(&notEmpty2, &lock);
        }
        pool = head1;
        head1 = pool->next;
        bufferLeft++;
        data2--;
        printf("Pop    2    [1]:%d    [2]:%d    总数:%d\n", data1, data2, MIX - bufferLeft);
        pthread_cond_signal(&notFull);
        pthread_mutex_unlock(&lock);
        MPMCQueueDestory(pool);
        sleep(rand() % 2);
    }
}

int main()
{
    pthread_t Push1, Push2, Pop1, Pop2;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&Push1, NULL, (void *)MPMCQueuePush1, NULL);
    pthread_create(&Push2, NULL, (void *)MPMCQueuePush2, NULL);
    pthread_create(&Pop1, NULL, (void *)MPMCQueuePop1, NULL);
    pthread_create(&Pop2, NULL, (void *)MPMCQueuePop2, NULL);

    pthread_join(Push1, NULL);
    pthread_join(Push2, NULL);
    pthread_join(Pop1, NULL);
    pthread_join(Pop2, NULL);

    pthread_mutex_destroy(&lock);
}

