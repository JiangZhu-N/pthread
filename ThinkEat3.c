#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

pthread_mutex_t lock;

sem_t chopsticks[5];
sem_t count;

void *ThinkEat(void *arg)
{
    int seat = *(char *)arg - 65;
    int Rchopstick, Lchopstick;
    Rchopstick = (seat + 1) % 5;
    Lchopstick = seat;

    while (1)
    {
        sem_wait(&count);

        printf("%c 正在思考\n", *(char *)arg);
        sleep(rand() % 3 + 1);
        sem_wait(&chopsticks[Rchopstick]);
        printf("%c 拿起 %d,暂时无法吃饭\n", *(char *)arg, Rchopstick);
        sem_wait(&chopsticks[Lchopstick]);
        printf("%c 拿起 %d,正在吃饭\n", *(char *)arg, Lchopstick);
        sleep(rand() % 3 + 1);
        sem_post(&chopsticks[Rchopstick]);
        sem_post(&chopsticks[Lchopstick]);

        sem_post(&count);

        printf("%c 放下 %d,%d,已退出\n", *(char *)arg, Rchopstick, Lchopstick);

        break;
    }
}

int main()
{

    pthread_t A, B, C, D, E;

    sem_init(&count, 0, 4);

    for (int i = 0; i < 5; i++)
    {
        sem_init(&chopsticks[i], 0, 1);
    }

    pthread_mutex_init(&lock, NULL);

    pthread_create(&A, NULL, ThinkEat, "A");
    pthread_create(&B, NULL, ThinkEat, "B");
    pthread_create(&C, NULL, ThinkEat, "C");
    pthread_create(&D, NULL, ThinkEat, "D");
    pthread_create(&E, NULL, ThinkEat, "E");

    pthread_join(A, NULL);
    pthread_join(B, NULL);
    pthread_join(C, NULL);
    pthread_join(D, NULL);
    pthread_join(E, NULL);

    printf("结束\n");

    // pthread_mutex_destroy(&lock);
}