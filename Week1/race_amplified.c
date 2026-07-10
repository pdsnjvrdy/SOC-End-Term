#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


#define ITERS 1000

/* A SHARED global counter. Both threads will modify this. */
long counter = 0;

long long binary_search(long long key,long long low, long long high) {
    if(low > high) return -1;
    long long mid = low +(high-low)/2;
    if(mid == key) return 1;
    else if(key > mid) {
        return binary_search(key,mid+1,high);
    }else {
        return binary_search(key,low,mid-1);
    }
}

void incrementar(long long i) {
    if(i > ITERS) {
        return;
    }
    else {
        long long temp = counter;
        {
            long long low = i/2;
            long long high = i*2;
            long long random_number = rand() % (high - low + 1) + low;
            int nothing = binary_search(random_number,1,i);
            temp = temp + 0*nothing;
        }
        temp = temp + 1;
        {
            long long low = i/2;
            long long high = i*2;
            long long random_number = rand() % (high - low + 1) + low;
            int nothing = binary_search(random_number,1,i);
            temp = temp + 0*nothing;
        }
        counter = temp;
        incrementar(i+1);
    }
}

void *increment(void *arg) {
    (void)arg;         /* unused — pthread signature requires it */
    incrementar(1);
    return NULL;
}

int main(void) {
    srand(time(NULL));
    pthread_t t1, t2;

    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    long expected = 2L * ITERS;
    printf("Final counter:  %ld\n", counter);
    printf("Expected:       %ld\n", expected);
    printf("Difference:     %ld\n", expected - counter);

    if (counter == expected) {
        printf("Result: CORRECT (but try running it again, you might get unlucky)\n");
    } else {
        printf("Result: WRONG by %ld — race condition observed.\n", expected - counter);
    }

    return 0;
}
