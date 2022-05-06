#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

pthread_mutex_t lock;

sem_t chopsticks[5];

void *ThinkEat(void *arg)
{
    int seat = *(char *)arg - 65;
    int Rchopstick, Lchopstick;
    Rchopstick = (seat + 1) % 5;
    Lchopstick = seat;

    while (1)
    {
        sleep(rand() % 2);

        pthread_mutex_lock(&lock);
        if (sem_trywait(&chopsticks[Rchopstick]) == 0)
        {
            if (sem_trywait(&chopsticks[Lchopstick]) == 0)
            {

                printf("%c 拿起 %d,%d,正在吃饭\n", *(char *)arg, Rchopstick, Lchopstick);
                pthread_mutex_unlock(&lock);
                sleep(rand() % 2);

                sem_post(&chopsticks[Rchopstick]);
                sem_post(&chopsticks[Lchopstick]);

                printf("%c 放下 %d,%d\n", *(char *)arg, Rchopstick, Lchopstick);
                break;
            }
            else
            {

                sem_post(&chopsticks[Rchopstick]);
                pthread_mutex_unlock(&lock);
            }
        }
        else
        {
            pthread_mutex_unlock(&lock);
        }
    }
}

int main()
{

    pthread_t A, B, C, D, E;

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