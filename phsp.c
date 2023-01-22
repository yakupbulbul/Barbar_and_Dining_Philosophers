#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>

#define MIN_THINK 1
#define MAX_THINK 60000
#define MIN_DINE 1
#define MAX_DINE 60000

int num_philosophers;
int min_think, max_think;
int min_dine, max_dine;
char* dst;
int num_dine;
pthread_mutex_t chopsticks[27];
double hungry_time[27];

double rand_exp(double mean) {
    double u = rand() / (RAND_MAX + 1.0);
    return -mean * log(u);
}

void *philosopher(void *arg) {
    int id = (intptr_t)arg;
    int left = id;
    int right = (id + 1) % num_philosophers;
    int i;
    for (i = 0; i < num_dine; i++) {
        int thinking_time;
        if (strcmp(dst
    , "uniform") == 0) {
            thinking_time = rand() % (max_think - min_think + 1) + min_think;
        } else if (strcmp(dst, "exponential") == 0) {
            double mean = (min_think + max_think) / 2.0;
            thinking_time = (int)rand_exp(mean);
            while (thinking_time < min_think || thinking_time > max_think) {
                thinking_time = (int)rand_exp(mean);
            }
        }

        struct timespec sleep_time;
        sleep_time.tv_sec = thinking_time/1000;
        sleep_time.tv_nsec = (thinking_time % 1000) * 1000000;
        nanosleep(&sleep_time, NULL);

        // Start measuring hungry time
        clock_t start = clock();
        pthread_mutex_lock(&chopsticks[left]);
        pthread_mutex_lock(&chopsticks[right]);

        int dining_time;
        if (strcmp(dst, "uniform") == 0) {
            dining_time = rand() % (max_dine - min_dine + 1) + min_dine;
        } else if (strcmp(dst, "exponential") == 0) {
            double mean = (min_dine + max_dine) / 2.0;
            dining_time = (int)rand_exp(mean);
            while (dining_time < min_dine || dining_time > max_dine) {
                dining_time = (int)rand_exp(mean);
            }
        }

        sleep_time.tv_sec = dining_time/1000;
        sleep_time.tv_nsec = (dining_time % 1000) * 1000000;
        nanosleep(&sleep_time, NULL);

        pthread_mutex_unlock(&chopsticks[left]);
        pthread_mutex_unlock(&chopsticks[right]);

        // Stop measuring hungry time
        clock_t end = clock();
        double duration = (double)(end - start) / CLOCKS_PER_SEC;
        hungry_time[id] += duration;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 8) {
        printf("Usage: %s <num_phsp> <min_think> <max_think> <min_dine> <max_dine> <dst> <num>\n", argv[0]);
        return 1;
    }

    // Read input from command line
    num_philosophers = atoi(argv[1]);
    min_think = atoi(argv[2]);
    max_think = atoi(argv[3]);
    min_dine = atoi(argv[4]);
    max_dine = atoi(argv[5]);
    dst = argv[6];
    num_dine = atoi(argv[7]);

    // Initialize chopsticks
    int i;
    for (i = 0; i < num_philosophers; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }

    // Create threads for philosophers
    pthread_t philosophers[num_philosophers];
    for (i = 0; i < num_philosophers; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, (void *)(intptr_t)i);
    }

    // Wait for all philosophers to finish dining
    for (i = 0; i < num_philosophers; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Calculate average hungry time
    double total_hungry_time = 0;
    for (i = 0; i < num_philosophers; i++) {
        total_hungry_time += hungry_time[i];
    }
    double average_hungry_time = total_hungry_time / num_philosophers;

    // Calculate standard deviation of hungry time
    double sum_of_squared_differences = 0;
    for (i = 0; i < num_philosophers; i++) {
        double difference = hungry_time[i] - average_hungry_time;
        sum_of_squared_differences += difference * difference;
    }
    double standard_deviation = sqrt(sum_of_squared_differences / num_philosophers);

    // Print results
    printf("Average hungry time: %f\n", average_hungry_time);
    printf("Standard deviation of hungry time: %f\n", standard_deviation);

    // Clean up chopsticks
    for (i = 0; i < num_philosophers; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }

    return 0;
}
