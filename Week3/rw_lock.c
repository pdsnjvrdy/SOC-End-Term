#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int reader_count = 0;
int writer_active = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t write_cond = PTHREAD_COND_INITIALIZER;

void reader_lock(void) {
    pthread_mutex_lock(&lock);
    while (writer_active) {
        pthread_cond_wait(&read_cond, &lock);
    }
    reader_count++;
    pthread_mutex_unlock(&lock);
}

void reader_unlock(void) {
    pthread_mutex_lock(&lock);
    reader_count--;
    if (reader_count == 0) {
        // No more readers, wake up a waiting writer (if any)
        pthread_cond_signal(&write_cond);
    }
    pthread_mutex_unlock(&lock);
}

void writer_lock(void) {
    pthread_mutex_lock(&lock);
    while (writer_active || reader_count > 0) {
        pthread_cond_wait(&write_cond, &lock);
    }
    writer_active = 1;
    pthread_mutex_unlock(&lock);
}

void writer_unlock(void) {
    pthread_mutex_lock(&lock);
    writer_active = 0;
    pthread_cond_broadcast(&read_cond);
    pthread_cond_signal(&write_cond);
    pthread_mutex_unlock(&lock);
}

int shared_value = 0;

void *reader_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        reader_lock();
        printf("Reader %d reads value = %d\n", id, shared_value);
        usleep(100000);   // simulate reading work
        reader_unlock();
        usleep(50000);
    }
    return NULL;
}

void *writer_thread(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 3; i++) {
        writer_lock();
        shared_value++;
        printf("Writer %d increments value to %d\n", id, shared_value);
        usleep(200000);   // simulate writing work
        writer_unlock();
        usleep(100000);
    }
    return NULL;
}

int main(void) {
    pthread_t readers[5], writers[2];
    int r_ids[5], w_ids[2];

    for (int i = 0; i < 5; i++) {
        r_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader_thread, &r_ids[i]);
    }
    for (int i = 0; i < 2; i++) {
        w_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer_thread, &w_ids[i]);
    }
    for (int i = 0; i < 5; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < 2; i++) {
        pthread_join(writers[i], NULL);
    }

    printf("Final shared value: %d\n", shared_value);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&read_cond);
    pthread_cond_destroy(&write_cond);

    return 0;
}