#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_PRODUCERS 20
#define NUM_CONSUMERS 20
#define ITEMS_PER_PRODUCER 1000

int *buffer = NULL;
int capacity;
int in = 0;
int out = 0;
int curr_size = 0;

long long total_produced = 0;
long long total_consumed = 0;
long long produced_value_sum = 0;
long long consumed_value_sum = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void buffer_init(int cap) {
    capacity = cap;
    buffer = malloc(capacity * sizeof(int));
    if(buffer == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
}

void buffer_put(int item) {
    pthread_mutex_lock(&lock);
    while(curr_size == capacity) {
        pthread_cond_wait(&not_full, &lock);
    }

    buffer[in] = item;
    in = (in + 1) % capacity;
    curr_size++;
    total_produced++;
    produced_value_sum += item;

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
}

int buffer_get(void) {
    pthread_mutex_lock(&lock);
    while(curr_size == 0) {
        pthread_cond_wait(&not_empty, &lock);
    }

    int item = buffer[out];
    out = (out + 1) % capacity;
    curr_size--;
    total_consumed++;
    consumed_value_sum += item;

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);

    return item;
}

void buffer_destroy(void) {
    free(buffer);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
}

void *producer(void *arg) {
    int id = *(int *)arg;
    for(int i = 0; i < ITEMS_PER_PRODUCER; i++){
        int item = id * 1000 + i;
        buffer_put(item);
    }
    return NULL;
}

void *consumer(void *arg){
    int items_to_consume =
        (NUM_PRODUCERS * ITEMS_PER_PRODUCER) / NUM_CONSUMERS;

    long long local_sum = 0;
    for(int i = 0; i < items_to_consume; i++) {
        int item = buffer_get();
        local_sum += item;
    }
    return NULL;
}

int main()
{
    printf("Enter buffer capacity: ");
    scanf("%d", &capacity);
    buffer_init(capacity);

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    int producer_ids[NUM_PRODUCERS];
    for(int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        pthread_create( &producers[i], NULL, producer, &producer_ids[i]);
    }

    for(int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create( &consumers[i], NULL, consumer, NULL);
    }

    for(int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    for(int i = 0; i < NUM_CONSUMERS; i++){
        pthread_join(consumers[i], NULL);
    }

    printf("\nTotal Produced : %lld\n", total_produced);
    printf("Total Consumed : %lld\n", total_consumed);
    printf("Produced Value Sum : %lld\n", produced_value_sum);

    printf("Consumed Value Sum : %lld\n", consumed_value_sum);

    if(total_produced == total_consumed && produced_value_sum == consumed_value_sum) printf("\nVERIFICATION PASSED\n");
    else printf("\nVERIFICATION FAILED\n");

    buffer_destroy();
    return 0;
}