#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define MAX_CUSTOMERS 27

#define MAX_WAITING_CHAIRS 5

// Mutex and condition variable to synchronization of access to the buffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t barber_sleep = PTHREAD_COND_INITIALIZER;

pthread_cond_t customer_done = PTHREAD_COND_INITIALIZER;

// Struct to representing of a customer
struct customer {
    int id;
    time_t arrive_time;
};

// Buffer (waiting chairs) implementation as a circular queue
struct customer waiting_customers[MAX_WAITING_CHAIRS];

int head = 0, tail = 0, waiting_count = 0;

// Function to adding a customer to the buffer
void add_customer_to_buffer(int id, time_t arrive_time) {

    waiting_customers[tail].id = id;

    waiting_customers[tail].arrive_time = arrive_time;
    tail = (tail + 1) % MAX_WAITING_CHAIRS;
    waiting_count++;
}

// Function to geting the next customer from the buffer
struct customer get_next_customer() {
    struct customer next = waiting_customers[head];


    head = (head + 1) % MAX_WAITING_CHAIRS;


    waiting_count--;
    return next;
}

// Function to checking if the buffer is empty
int buffer_is_empty() {
    return waiting_count == 0;
}

// Function to checking if the buffer is full
int buffer_is_full() {
    return waiting_count == MAX_WAITING_CHAIRS;
}

// Function for the barber thread
void* barber_thread(void* arg) {
    while (1) {

        pthread_mutex_lock(&mutex);

        while (buffer_is_empty()) {
            printf("Barber: sleeping\n");
            pthread_cond_wait(&barber_sleep, &mutex);
        }
        struct customer next_customer = get_next_customer();

        pthread_mutex_unlock(&mutex);

        printf("Barber: cutting hair of Customer %d\n", next_customer.id);

        struct timespec duration;

        duration.tv_sec = 0;

        duration.tv_nsec = (*((int*) arg)) * 1000000;

        nanosleep(&duration, NULL);

        pthread_cond_signal(&customer_done);

    }
    return NULL;
}

// Function for the customer thread
void* customer_thread(void* arg) {

       int id = *((int*) arg);
    time_t arrive_time;
    int haircuts = *((int*) (arg + sizeof(int)));
    int wait_time;


    while (haircuts--) {
        pthread_mutex_lock(&mutex);
        arrive_time = time(NULL);


        if (buffer_is_full()) {
            printf("Customer %d: Leaving\n", id);
            pthread_mutex_unlock(&mutex);


            struct timespec duration;
            duration.tv_sec = 0;


            duration.tv_nsec = (*((int*) (arg + sizeof(int) * 2))) * 1000000;


            nanosleep(&duration, NULL);
            continue;
        }
        add_customer_to_buffer(id, arrive_time);
        printf("Customer %d: Entering shop, waiting\n", id);

        pthread_cond_signal(&barber_sleep);
        pthread_cond_wait(&customer_done, &mutex);


        wait_time = difftime(time(NULL), arrive_time);


        printf("Customer %d: Waited %d seconds\n", id, wait_time);


        pthread_mutex_unlock(&mutex);
        struct timespec duration;
        duration.tv_sec = 0;

        duration.tv_nsec = (*((int*) (arg + sizeof(int) * 2))) * 1000000;
        
        nanosleep(&duration, NULL);
    }
    return NULL;
}

int main(int argc, char* argv[]) {

    if (argc < 5) {
        fprintf(stderr, "Usage: barber <num_customer> <max_arrival_time> <max_haricut_duration> <haricut_repetion>\n");
        return 1;
    }
    int num_of_customers = atoi(argv[1]);


    int max_arrival_time = atoi(argv[2]);

    int max_haircut_duration = atoi(argv[3]);

    int haircut_repetition = atoi(argv[4]);

    if (num_of_customers > MAX_CUSTOMERS) {
        fprintf(stderr, "Error: Max number of customers is %d\n", MAX_CUSTOMERS);
        return 1;
    }
    int customer_args[num_of_customers][3];


    pthread_t barber, customers[num_of_customers];

    pthread_create(&barber, NULL, barber_thread, &max_haircut_duration);
    for (int i = 0; i < num_of_customers; i++) {

        customer_args[i][0] = i + 1;

        customer_args[i][1] = haircut_repetition;

        customer_args[i][2] = max_arrival_time;
        pthread_create(&customers[i], NULL, customer_thread, customer_args[i]);
    }
    for (int i = 0; i < num_of_customers; i++) {


        pthread_join(customers[i], NULL);
    }
    return 0;
}

