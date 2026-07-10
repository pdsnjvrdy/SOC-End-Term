#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define NUM_PRODUCERS 10
#define NUM_CONSUMERS 10
#define ITEMS_PER_PRODUCER 1000

int *buffer;
int capacity;
int in = 0;
int out = 0;

long total_produced = 0;
long total_consumed = 0;
long produced_sum = 0;
long consumed_sum = 0;

sem_t empty_slots;
sem_t full_slots;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void buffer_init(int cap) {
    capacity = cap;
    buffer = malloc(capacity * sizeof(int));

    sem_init(&empty_slots, 0, capacity);
    sem_init(&full_slots, 0, 0);
}

void buffer_put(int item) {
    sem_wait(&empty_slots);
    pthread_mutex_lock(&mutex);

    buffer[in] = item;
    in = (in + 1) % capacity;
    total_produced++;
    produced_sum += item;

    pthread_mutex_unlock(&mutex);
    sem_post(&full_slots);
}

int buffer_get(void) {
    sem_wait(&full_slots);
    pthread_mutex_lock(&mutex);

    int item = buffer[out];
    out = (out + 1) % capacity;
    total_consumed++;
    consumed_sum += item;

    pthread_mutex_unlock(&mutex);
    sem_post(&empty_slots);
    return item;
}

void buffer_destroy(void) {
    free(buffer);
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
}

void *producer(void *arg) {
    int id = *(int *)arg;
    long *count = malloc(sizeof(long));
    *count = 0;

    for(int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * ITEMS_PER_PRODUCER + i;
        buffer_put(item);
        (*count)++;
    }
    return count;
}

void *consumer(void *arg) {
    int items_to_consume = (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;
    long *count = malloc(sizeof(long));
    *count = 0;

    for(int i = 0; i < items_to_consume; i++) {
        buffer_get();
        (*count)++;
    }
    return count;
}

int main() {
    int cap;

    printf("Enter buffer capacity: ");
    scanf("%d", &cap);

    buffer_init(cap);
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    int producer_ids[NUM_PRODUCERS];
    for(int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        pthread_create( &producers[i], NULL, producer, &producer_ids[i]);
    }

    for(int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    long producer_count = 0;
    long consumer_count = 0;

    for(int i = 0; i < NUM_PRODUCERS; i++) {
        long *result;
        pthread_join(producers[i], (void **)&result);
        producer_count += *result;
        free(result);
    }

    for(int i = 0; i < NUM_CONSUMERS; i++) {
        long *result;
        pthread_join(consumers[i], (void **)&result);
        consumer_count += *result;
        free(result);
    }

    printf("\nTotal Produced : %ld\n", total_produced);
    printf("Total Consumed : %ld\n", total_consumed);
    printf("Producer Count : %ld\n", producer_count);
    printf("Consumer Count : %ld\n", consumer_count);
    printf("Produced Sum   : %ld\n", produced_sum);
    printf("Consumed Sum   : %ld\n", consumed_sum);

    if(total_produced == total_consumed &&
       producer_count == consumer_count &&
       produced_sum == consumed_sum)
    {
        printf("\nVERIFICATION PASSED\n");
    }
    else
    {
        printf("\nVERIFICATION FAILED\n");
    }

    buffer_destroy();
    pthread_mutex_destroy(&mutex);
    return 0;
}