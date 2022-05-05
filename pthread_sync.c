
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
//缓冲区
#define MIX 10

struct SPSCQueue
{
    /* Define Your Data Here */
    struct SPSCQueue *next;
    void *Data;
} typedef SPSCQueue;

SPSCQueue *head;

pthread_mutex_t lock;
pthread_cond_t notEmpty;
int bufferLeft = MIX;

SPSCQueue *SPSCQueueInit()
{
    SPSCQueue *head;
    head = malloc(sizeof(SPSCQueue));
    return head;
}

void SPSCQueueDestory(SPSCQueue *pool)
{
    free(pool);
}

void SPSCQueuePush(SPSCQueue *pool, void *s)
{
    while (1)
    {

        if (bufferLeft > 0)
        {
            pthread_mutex_lock(&lock);
            pool = SPSCQueueInit(pool);
            pool->Data = s;
            pool->next = head;
            head = pool;
            bufferLeft--;
            printf("Push    总数:%d\n", MIX - bufferLeft);
            pthread_mutex_unlock(&lock);
            pthread_cond_signal(&notEmpty);
        }
        else
        {
            printf("Push失败:缓冲区已满\n");
        }
        sleep(rand() % 2);
    }
}

void *SPSCQueuePop(SPSCQueue *pool)
{
    while (1)
    {

        while (bufferLeft >= MIX)
        {
            if (bufferLeft >= MIX)
            {
                printf("Pop失败:缓冲区为空\n");
            }
            pthread_cond_wait(&notEmpty, &lock);
        }
        pool = head;
        head = pool->next;
        bufferLeft++;
        pthread_mutex_unlock(&lock);
        printf("Pop     总数:%d\n", MIX - bufferLeft);
        SPSCQueueDestory(pool);
        sleep(rand() % 2);
    }
}

int main()
{
    pthread_t Push, Pop;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&Push, NULL, (void *)SPSCQueuePush, NULL);
    pthread_create(&Pop, NULL, (void *)SPSCQueuePop, NULL);

    pthread_join(Push, NULL);
    pthread_join(Pop, NULL);
}
