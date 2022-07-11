#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//改变线程数量
#define CHANGENUM 2

//任务体结构
typedef struct Task
{
    void (*fun)(void *arg);
    void *arg;
} Task;
//线程池结构体
typedef struct Threadpool
{
    //任务队列
    Task *Taskqueue;
    int Qnum;  //当前任务个数
    int Qsize; //池中任务个数
    int Qhand; //队头
    int Qtail; //队尾

    //线程池
    pthread_t managerID;
    pthread_t *workerID;
    pthread_mutex_t lockpool;
    pthread_mutex_t lockbusyNum;
    pthread_cond_t notFull;
    pthread_cond_t notEmpty;

    int minNum;
    int maxNum;
    int busyNum;
    int liveNum;
    int exitNum;

    int exit; // 1为销毁线程池

} Threadpool;

//函数
void *worker(void *arg);
void *manager(void *arg);
void threadExit(Threadpool *pool);
void addTask(Threadpool *pool, void (*func)(void *), void *arg);
int workingThreadNum(Threadpool *pool);
int livingingThreadNum(Threadpool *pool);
int destorypool(Threadpool *pool);

//创建线程池并初始化
Threadpool *threadpoolCreat(int minNum, int maxNum, int Qsize)
{
    Threadpool *pool = (Threadpool *)malloc(sizeof(Threadpool));
    pool->workerID = (pthread_t *)malloc(sizeof(pthread_t) * maxNum);

    memset(pool->workerID, 0, sizeof(pthread_t) * maxNum);

    pool->maxNum = maxNum;
    pool->minNum = minNum;
    pool->busyNum = 0;
    pool->liveNum = minNum;
    pool->exitNum = 0;

    pthread_mutex_init(&pool->lockpool, NULL);
    pthread_mutex_init(&pool->lockbusyNum, NULL);
    pthread_cond_init(&pool->notFull, NULL);
    pthread_cond_init(&pool->notEmpty, NULL);

    //任务队列
    pool->Taskqueue = (Task *)malloc(sizeof(Task) * Qsize);
    pool->Qsize = Qsize;
    pool->Qnum = 0;
    pool->Qhand = 0;
    pool->Qtail = 0;

    pool->exit = 0;

    //创建线程
    pthread_create(&pool->managerID, NULL, manager, pool);
    for (int i = 0; i < minNum; ++i)
    {
        pthread_create(&pool->workerID[i], NULL, worker, pool);
    }

    return pool;
    /* //释放内存
    if(pool&&pool->workerID) free(pool->workerID);
    if(pool&&pool->Taskqueue) free(pool->Taskqueue);
    if(pool) free(pool); */
}

// worker
void *worker(void *arg)
{
    Threadpool *pool = (Threadpool *)arg;
    while (1)
    {
        pthread_mutex_lock(&pool->lockpool);
        while (pool->Qnum == 0 && !pool->exit)
        {
            pthread_cond_wait(&pool->notEmpty, &pool->lockpool);
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum)
                {

                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->lockpool);
                    threadExit(pool);
                                }
            }
        }
        if (pool->exit)
        {
            pthread_mutex_unlock(&pool->lockpool);
            threadExit(pool);
        }

        //取出任务
        Task t;
        t.fun = pool->Taskqueue[pool->Qhand].fun;
        t.arg = pool->Taskqueue[pool->Qhand].arg;
        //移动头结点
        pool->Qhand = (pool->Qhand + 1) % pool->Qsize;
        pool->Qnum--;

        pthread_cond_signal(&pool->notFull);
        pthread_mutex_unlock(&pool->lockpool);

        printf("线程%ld正在工作\n", pthread_self());

        //执行任务
        pthread_mutex_lock(&pool->lockbusyNum);
        pool->busyNum++;
        pthread_mutex_unlock(&pool->lockbusyNum);

        t.fun(t.arg);
        free(t.arg);
        t.arg = NULL;

        printf("线程%ld结束工作\n", pthread_self());

        pthread_mutex_lock(&pool->lockbusyNum);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->lockbusyNum);

        pthread_mutex_unlock(&pool->lockpool);
    }
}
// manager
void *manager(void *arg)
{
    Threadpool *pool = (Threadpool *)arg;
    while (!pool->exit)
    {

        sleep(3);

        //取出线程池数据
        pthread_mutex_lock(&pool->lockpool);
        int Qnum = pool->Qnum;
        int liveNum = pool->liveNum;
        pthread_mutex_unlock(&pool->lockpool);

        pthread_mutex_lock(&pool->lockbusyNum);
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->lockbusyNum);

        //添加线程
        if (Qnum > liveNum && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->lockpool);
            int c = 0;

            for (int i = 0; i < pool->maxNum && c < CHANGENUM && pool->liveNum < pool->maxNum; ++i)
            {
                if (pool->workerID[i] == 0)
                {
                    pthread_create(&pool->workerID[i], NULL, worker, pool);
                    c++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->lockpool);
        }

        //销毁线程
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->lockpool);
            pool->exitNum = CHANGENUM;
            pthread_mutex_unlock(&pool->lockpool);

            for (int i = 0; i < CHANGENUM; i++)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return NULL;
}

void threadExit(Threadpool *pool)
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < pool->maxNum; i++)
    {
        if (pool->workerID[i] == tid)
        {
            pool->workerID[i] = 0;
            printf("线程%ld已退出\n", tid);

            break;
        }
    }

    pthread_exit(NULL);
}

void addTask(Threadpool *pool, void (*func)(void *), void *arg)
{
    pthread_mutex_lock(&pool->lockpool);
    while (pool->Qnum == pool->Qsize && !pool->exit)
    {
        pthread_cond_wait(&pool->notFull, &pool->lockpool);
    }
    if (pool->exit)
    {
        pthread_mutex_unlock(&pool->lockpool);
        return;
    }

    //添加任务
    pool->Taskqueue[pool->Qtail].fun = func;
    pool->Taskqueue[pool->Qtail].arg = arg;
    pool->Qtail = (pool->Qtail + 1) % pool->Qsize;
    pool->Qnum++;

    pthread_cond_signal(&pool->notEmpty);

    pthread_mutex_unlock(&pool->lockpool);
}

int workingThreadNum(Threadpool *pool)
{
    pthread_mutex_lock(&pool->lockbusyNum);
    int busyNum = pool->busyNum;
    pthread_mutex_unlock(&pool->lockbusyNum);
    return busyNum;
}
int livingingThreadNum(Threadpool *pool)
{
    pthread_mutex_lock(&pool->lockpool);
    int livingNum = pool->liveNum;
    pthread_mutex_unlock(&pool->lockpool);
    return livingNum;
}
int destorypool(Threadpool *pool)
{
    if (pool == NULL)
    {
        return -1;
    }

    pool->exit = 1;
    pthread_join(pool->managerID, NULL);
    for (int i = 0; i < pool->liveNum; i++)
    {
        pthread_cond_signal(&pool->notEmpty);
    }
    if (pool->Taskqueue)
    {
        free(pool->Taskqueue);
    }
    if (pool->workerID)
    {
        free(pool->workerID);
    }

    pthread_mutex_destroy(&pool->lockpool);
    pthread_mutex_destroy(&pool->lockbusyNum);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);

    free(pool);
    pool = NULL;

    return 0;
}

//测试线程池
//任务函数
void taskFun(void *arg)
{
    int count = *(int *)arg;
    printf("count:%ld,id:%ld\n", count, pthread_self());

    sleep(1);
}
int main()
{
    //创建线程池
    Threadpool *pool = threadpoolCreat(3, 10, 100);

    for (int i = 0; i < 100; i++)
    {
        int *count = (int *)malloc(sizeof(int));
        *count = i;
        addTask(pool, taskFun, count);
    }
    sleep(30);
    destorypool(pool);
    printf("结束");
    return 0;
}
