// sum_worker.c
// Name: Yannick Fenz'l
// Matrikelnummer: ***REMOVED***
// Date: 07.12.2024

#include "sum.h"
#include <mqueue.h>  // POSIX message queues
#include <fcntl.h>   // file control constants
#include <sys/mman.h> // memory mapping
#include <semaphore.h> // semaphores
#include <stdio.h>   // standard I/O
#include <stdlib.h>  // standard library functions
#include <unistd.h>  // POSIX constants and functions

int main() {
    // Open the POSIX message queue
    mqd_t mq = mq_open(MQ_NAME, O_RDWR);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return EXIT_FAILURE;
    }

    // Open shared memory for accessing global sum structure
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }

    // Map shared memory to process's address space
    struct global_sum *sum_ptr = mmap(0, sizeof(struct global_sum), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sum_ptr == MAP_FAILED) {
        perror("mmap failed");
        return EXIT_FAILURE;
    }

    // Open semaphore for synchronization
    sem_t *sem = sem_open(SEM_NAME, O_RDWR);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        return EXIT_FAILURE;
    }

    long total_requests = 0; // Count total number of processed requests

    while (1) {
        struct msg_request request;

        // Receive a task from the message queue
        if (mq_receive(mq, (char *)&request, sizeof(request), NULL) == -1) {
            perror("mq_receive failed");
            return EXIT_FAILURE;
        }

        // Check if the request signals termination
        if (request.start == TERMINATION_SIGNAL) {
            break;
        }

        // Calculate the partial sum for the given range
        long partial_sum = 0;
        for (long i = request.start; i <= request.end; i++) {
            partial_sum += i;
        }

        // Update the global sum safely using a semaphore
        sem_wait(sem);
        sum_ptr->total += partial_sum;
        sem_post(sem);

        total_requests++; // Increment the count of processed requests
    }

    // Log the total number of processed requests by this worker
    printf("pid %d done after %ld requests\n", getpid(), total_requests);

    // Clean up resources
    munmap(sum_ptr, sizeof(struct global_sum));

    return EXIT_SUCCESS;
}
