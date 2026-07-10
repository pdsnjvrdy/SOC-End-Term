#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Task structure (linked list) 
typedef struct Task {
    void (*function)(void *);
    void *arg;
    struct Task *next;
} Task;

Task *task_head = NULL;
Task *task_tail = NULL;
int task_count = 0;

// Pool State
int num_workers = 0;
pthread_t *workers = NULL;
int shutdown_flag = 0;   // 1 when pool is being destroyed
int pending_tasks = 0;          

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_cond = PTHREAD_COND_INITIALIZER; 
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;   

void *worker_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        while (task_count == 0 && !shutdown_flag) {
            pthread_cond_wait(&work_cond, &lock);
        }

        if (shutdown_flag && task_count == 0) {
            pthread_mutex_unlock(&lock);
            break;
        }

        Task *task = task_head;
        if (task != NULL) {
            task_head = task->next;
            if (task_head == NULL) {
                task_tail = NULL;
            }
            task_count--;
        }
        pthread_mutex_unlock(&lock);

        if (task != NULL) {
            task->function(task->arg);

            pthread_mutex_lock(&lock);
            pending_tasks--;
            if (pending_tasks == 0) {
                pthread_cond_signal(&done_cond);
            }
            pthread_mutex_unlock(&lock);

            free(task);
        }
    }
    return NULL;
}

void pool_init(int num) {
    num_workers = num;
    workers = malloc(num_workers * sizeof(pthread_t));
    if (workers == NULL) {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < num_workers; i++) {
        pthread_create(&workers[i], NULL, worker_thread, NULL);
    }
}

void pool_submit(void (*function)(void *), void *arg) {
    Task *task = malloc(sizeof(Task));
    if (task == NULL) {
        perror("malloc");
        exit(1);
    }
    task->function = function;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&lock);
    // Append to queue
    if (task_tail == NULL) {
        task_head = task;
        task_tail = task;
    } else {
        task_tail->next = task;
        task_tail = task;
    }
    task_count++;
    pending_tasks++;
    pthread_cond_signal(&work_cond);
    pthread_mutex_unlock(&lock);
}

void pool_wait(void) {
    pthread_mutex_lock(&lock);
    while (pending_tasks > 0) {
        pthread_cond_wait(&done_cond, &lock);
    }
    pthread_mutex_unlock(&lock);
}

void pool_destroy(void) {
    // Signal all workers to exit
    pthread_mutex_lock(&lock);
    shutdown_flag = 1;
    pthread_cond_broadcast(&work_cond);
    pthread_mutex_unlock(&lock);

    // Wait for all workers to finish
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    free(workers);
    workers = NULL;

    // Clean up any remaining tasks (should be none if pool_wait was called)
    Task *curr = task_head;
    while (curr != NULL) {
        Task *next = curr->next;
        free(curr);
        curr = next;
    }
    task_head = NULL;
    task_tail = NULL;
    task_count = 0;
    pending_tasks = 0;

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&work_cond);
    pthread_cond_destroy(&done_cond);
}

int global_counter = 0;

void increment_task(void *arg) {
    int id = *(int *)arg;
    global_counter++;
    printf("Task %d: counter = %d\n", id, global_counter);
    usleep(50000);
}

int main(void) {
    pool_init(4);

    // Submit 20 tasks
    int ids[20];
    for (int i = 0; i < 20; i++) {
        ids[i] = i + 1;
        pool_submit(increment_task, &ids[i]);
    }

    pool_wait();

    printf("All tasks done. Final counter: %d (expected 20)\n", global_counter);

    pool_destroy();

    return 0;
}