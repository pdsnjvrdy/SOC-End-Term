#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include <stdbool.h>

const int people = 100;
long long eco_value = people*people;
long long total_transactions = 30000;
long long failed = 0;
long long successful = 0;
long long cheeky = 0;

typedef struct {
    int id;
    int balance;
} account;


void* transaction(void *arg) {
    account* ac = (account*)arg;
    int who = rand()%people;
    if(who == ac->id-1) {
        ++cheeky;
        return NULL;
    }
    int how_much = rand()%people+1;
    account* re = ac - ac->id + 1 + who;

    bool Credit = (rand() > ac->id) ? true : false;

    if(Credit) {
        if(re->balance - how_much >= 0) {
            ++successful;
            re->balance -= how_much;
            ac->balance += how_much;
        }
        else {
            ++failed;
        }
    }
    else {
        if(ac->balance - how_much >= 0) {
            ++successful;
            ac->balance -= how_much;
            re->balance += how_much;
        }
        else {
            ++failed;
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    account person[people];
    for(int i = 1; i <= people; ++i) {
        person[i-1].id = i;
        person[i-1].balance = people;
    }

    pthread_t thread[total_transactions];

    for(int i = 0; i < total_transactions; ++i) {
        int who = rand() % people;
        account *p = &person[who];
        pthread_create(&thread[i], NULL, transaction, p);
    }
    for(int i = 0; i < total_transactions; ++i) {
        pthread_join(thread[i], NULL);
    }
    long long value = 0;
    for(int i = 0; i < people; ++i) {
        value += person[i].balance;
    }
    printf("balance after billion transactions : %lld\nchange : %lld\n", value, eco_value-value);
    printf("successful transcations : %lld\n", successful);
    printf("failed transcations : %lld\n", failed);
    printf("total attemts to waste bank time : %lld\n" , cheeky);
}

