#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

typedef struct Node {
    int value;
    struct Node* next;
} Node;

Node* head;
Node* curr_node;

int deletes = 0;
int inserts = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void list_init(void) {
    head = NULL;
    curr_node = NULL;
}
void list_insert(int val) {
    if(head == NULL) {
        head = malloc(sizeof(Node));
        curr_node = head;
        curr_node->value = val;
    }
    else {
        curr_node->next = malloc(sizeof(Node));
        curr_node = curr_node->next;
        curr_node->value = val;
    }
    curr_node->next = NULL;

    ++inserts;
}
int list_contains(int val) {
    Node* temp = head;
    while(temp != NULL) {
        if(temp->value == val) return 1;
        temp = temp->next;
    }
    return 0;
}
void list_remove(int val) {
    if(curr_node == NULL) {
/*         printf("Element %d not found in the list\n", val); */
    }
    else if(curr_node == head) {
        if(head->value != val) {
 /*            printf("Element %d not found in the list\n", val); */
            return;
        }
   /*      printf("Element %d found in the list\n", val); */
        free(head);
        head = NULL;
        curr_node = NULL;
        ++deletes;
    }
    else {
        Node* temp = head;
            while(temp->next->value != val && temp->next->next != NULL) {
                temp = temp->next;
            }
            if(temp->next->value == val) {
               /*  printf("Element %d found in the list\n", val); */
                if(curr_node == temp->next) curr_node = temp;
                Node* victim = temp->next;
                temp->next = victim->next;
                free(victim);
                ++deletes;
            }
            else {
          /*       printf("Element %d not found in the list\n", val); */
            }
        }
    }

void list_destroy(void) {
    Node* temp = head;
    while(temp != NULL) {
        Node* next = temp->next;
        free(temp);
        temp = next;
    }
}

void* create_check_destroy(void* arg) {
    unsigned int seed = time(NULL) ^ (unsigned long)pthread_self();
    for(int i = 1; i <= 3000; ++i) {
        if(i%3 == 1) {
            pthread_mutex_lock(&lock);
            list_insert(rand_r(&seed)%100);
            pthread_mutex_unlock(&lock);
        }
        else if(i%3 == 2) {
            pthread_mutex_lock(&lock);
      /*       int val = rand_r(&seed)%100;
            if(list_contains(val)) printf("Found %d \n", val);
            else printf("Not found %d\n", val); */
            pthread_mutex_unlock(&lock);
        }
        else {
            pthread_mutex_lock(&lock);
            list_remove(rand_r(&seed)%100);
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

void* destroy_create_check(void* arg) {
    unsigned int seed = time(NULL) ^ (unsigned long)pthread_self();
    for(int i = 1; i <= 3000; ++i) {
        if(i%3 == 2) {
            pthread_mutex_lock(&lock);
            list_insert(rand_r(&seed)%100);
            pthread_mutex_unlock(&lock);
        }
        else if(i%3 == 0) {
            pthread_mutex_lock(&lock);
/*             int val = rand_r(&seed)%100;
            if(list_contains(val)) printf("Found %d \n", val);
            else printf("Not found %d\n", val); */
            pthread_mutex_unlock(&lock);
        }
        else {
            pthread_mutex_lock(&lock);
            list_remove(rand_r(&seed)%100);
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

void* check_destroy_create(void* arg) {
    unsigned int seed = time(NULL) ^ (unsigned long)pthread_self();

    for(int i = 1; i <= 3000; ++i) {
        if(i%3 == 0) {
            pthread_mutex_lock(&lock);
            list_insert(rand_r(&seed)%100);
            pthread_mutex_unlock(&lock);
        }
        else if(i%3 == 1) {
            pthread_mutex_lock(&lock);
/*             int val = rand_r(&seed)%100;
            if(list_contains(val)) printf("Found %d \n", val);
            else printf("Not found %d\n", val); */
            pthread_mutex_unlock(&lock);
        }
        else {
            pthread_mutex_lock(&lock);
            list_remove(rand_r(&seed)%100);
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int has_cycle(void) {
    Node* slow = head;
    Node* fast = head;

    while(fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;

        if(slow == fast)
            return 1;
    }

    return 0;
}


int main() {
    srand(time(NULL));
    int rand_r(unsigned int *seed);

    pthread_t THREAD[9];
    for(int i = 0; i < 9; ++i) {
        if(i%3 == 1) pthread_create(&THREAD[i],NULL,create_check_destroy,NULL);
        else if(i%3 == 2) pthread_create(&THREAD[i],NULL,check_destroy_create,NULL);
        else pthread_create(&THREAD[i],NULL,destroy_create_check,NULL);
    }
    for(int i = 0; i < 9; ++i) {
        if(i%3 == 1) pthread_join(THREAD[i],NULL);
        else if(i%3 == 2) pthread_join(THREAD[i], NULL);
        else pthread_join(THREAD[i], NULL);        
    }

    // Checking if the linked list is good
    int count = 0;
    Node* temp = head;
    while(temp != NULL && count <= 3*1000) {
        ++count;
        temp = temp->next;
    }
    if(count <= 3000 && 
        temp == NULL && 
        count == inserts - deletes  &&
        has_cycle() == 0
      ) printf("\n List is good \n");
    else printf("\n BUGG!!! \n");

    list_destroy();
    pthread_mutex_destroy(&lock);
    return 0;
}