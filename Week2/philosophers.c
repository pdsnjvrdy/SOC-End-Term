#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 

#define NUM_PHILOSOPHERS 5
#define EAT_TIMES        10

pthread_mutex_t forks[NUM_PHILOSOPHERS];
int eat_count[NUM_PHILOSOPHERS]; 

void *philosopher(void *arg) {
    int id = *(int *)arg;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;

    for (int i = 0; i < EAT_TIMES; i++) {
        printf("Philosopher %d is thinking\n", id);
        usleep(100000);   // 0.1 sec

        // Pick up forks – always acquire lower-numbered fork first
        int first, second;
        if (left < right) {
            first = left;
            second = right;
        } else {
            first = right;
            second = left;
        }

        printf("Philosopher %d tries fork %d (lower)\n", id, first);
        pthread_mutex_lock(&forks[first]);
        printf("Philosopher %d got fork %d\n", id, first);

        printf("Philosopher %d tries fork %d (higher)\n", id, second);
        pthread_mutex_lock(&forks[second]);
        printf("Philosopher %d got fork %d\n", id, second);

        eat_count[id]++;
        printf("Philosopher %d is eating (meal %d of %d)\n", id, eat_count[id], EAT_TIMES);
        usleep(200000);   // 0.2 sec

        // Put down forks (reverse order)
        pthread_mutex_unlock(&forks[second]);
        pthread_mutex_unlock(&forks[first]);
        printf("Philosopher %d put down forks\n", id);
    }

    printf("Philosopher %d finished all meals.\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
        eat_count[i] = 0;
    }
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
    }
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n--- Summary ---\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times\n", i, eat_count[i]);
    }
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    return 0;
}