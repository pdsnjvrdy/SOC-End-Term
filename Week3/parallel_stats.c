#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define ARRAY_SIZE 1000000
#define NUM_THREADS 5

typedef struct {
    int start;
    int end;
    int *array;
    long partial_sum;
    int partial_min;
    int partial_max;
} worker_result_t;

void *worker(void *arg) {
    worker_result_t *data = (worker_result_t *)arg;

    data->partial_sum = 0;
    data->partial_min = INT_MAX;
    data->partial_max = INT_MIN;

    for(int i = data->start; i < data->end; i++) {
        int value = data->array[i];
        data->partial_sum += value;
        if(value < data->partial_min) data->partial_min = value;
        if(value > data->partial_max) data->partial_max = value;
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    int *array = malloc(sizeof(int) * ARRAY_SIZE);
    if(array == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }
    for(int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 1000;
    }

    pthread_t threads[NUM_THREADS];
    worker_result_t results[NUM_THREADS];
    int chunk_size = ARRAY_SIZE / NUM_THREADS;

    for(int i = 0; i < NUM_THREADS; i++) {
        results[i].array = array;
        results[i].start = i * chunk_size;
        if(i == NUM_THREADS - 1) results[i].end = ARRAY_SIZE;
        else results[i].end = (i + 1) * chunk_size;

        pthread_create(&threads[i], NULL, worker, &results[i]);
    }

    long total_sum = 0;
    int global_min = INT_MAX;
    int global_max = INT_MIN;

    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);

        total_sum += results[i].partial_sum;
        if(results[i].partial_min < global_min) global_min = results[i].partial_min;
        if(results[i].partial_max > global_max) global_max = results[i].partial_max;
    }

    double parallel_avg = (double)total_sum / ARRAY_SIZE;

    // For Verfication

    long seq_sum = 0;
    int seq_min = INT_MAX;
    int seq_max = INT_MIN;

    for(int i = 0; i < ARRAY_SIZE; i++) {
        seq_sum += array[i];
        if(array[i] < seq_min) seq_min = array[i];
        if(array[i] > seq_max) seq_max = array[i];
    }

    double seq_avg = (double)seq_sum / ARRAY_SIZE;

    printf("Sequential Statistics  \n");
    printf("Sum     : %ld\n", seq_sum);
    printf("Minimum : %d\n", seq_min);
    printf("Maximum : %d\n", seq_max);
    printf("Average : %.2f\n\n", seq_avg);

    printf("Parallel Statistics   \n");
    printf("Sum     : %ld\n", total_sum);
    printf("Minimum : %d\n", global_min);
    printf("Maximum : %d\n", global_max);
    printf("Average : %.2f\n\n", parallel_avg);

    if(seq_sum == total_sum &&
       seq_min == global_min &&
       seq_max == global_max)
    {
        printf("VERIFICATION PASSED\n");
    }
    else
    {
        printf("VERIFICATION FAILED\n");
    }

    free(array);
    return 0;
}