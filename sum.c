#include "sum.h"
#include <sys/wait.h> // For wait() to handle child processes
#include <mqueue.h>   // For POSIX message queues
#include <fcntl.h>    // For file control constants
#include <sys/mman.h> // For shared memory
#include <semaphore.h> // For semaphores
#include <stdio.h>    // For standard I/O
#include <stdlib.h>   // For standard library functions
#include <unistd.h>   // For fork(), execlp(), and other POSIX functions

#define MAX_MSG_SIZE sizeof(struct msg_request) // Maximum size of a message

int main(int argc, char *argv[]) {
    // Validate input arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s n chunksize workers\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n = atol(argv[1]);         // Total range to sum
    long chunksize = atol(argv[2]); // Size of each chunk
    int workers = atoi(argv[3]);    // Number of worker processes

    // Validate argument ranges
    if (n <= 0 || chunksize < 1 || chunksize > MAX_CHUNKSIZE || workers < 1 || workers > MAX_WORKERS) {
        fprintf(stderr, "Invalid parameters.\n");
        return EXIT_FAILURE;
    }

    // Initialize the message queue
    struct mq_attr attr;
    attr.mq_flags = 0; // Blocking mode
    attr.mq_maxmsg = 10; // Maximum number of messages
    attr.mq_msgsize = MAX_MSG_SIZE; // Maximum message size
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open("/sum_queue", O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return EXIT_FAILURE;
    }

    // Create shared memory for storing the global sum
    int shm_fd = shm_open("/global_sum", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return EXIT_FAILURE;
    }

    // Set the size of shared memory
    if (ftruncate(shm_fd, sizeof(struct global_sum)) == -1) {
        perror("ftruncate failed");
        return EXIT_FAILURE;
    }

    // Map shared memory to the process's address space
    struct global_sum *sum_ptr = mmap(0, sizeof(struct global_sum), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sum_ptr == MAP_FAILED) {
        perror("mmap failed");
        return EXIT_FAILURE;
    }
    sum_ptr->total = 0; // Initialize the global sum to 0

    // Create a semaphore for synchronization
    sem_t *sem = sem_open("/sem", O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        return EXIT_FAILURE;
    }

    // Fork worker processes
    for (int i = 0; i < workers; i++) {
        if (fork() == 0) {
            // Replace the child process with the worker executable
            execlp("./sum_worker", "sum_worker", NULL);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        }
    }

    // Divide the range into chunks and send tasks to the workers
    for (long start = 1; start <= n; start += chunksize) {
        long end = (start + chunksize - 1 > n) ? n : start + chunksize - 1;

        struct msg_request request;
        request.start = start;
        request.end = end;

        if (mq_send(mq, (const char *)&request, sizeof(request), 0) == -1) {
            perror("mq_send failed");
            return EXIT_FAILURE;
        }
    }

    // Send termination signals to all workers
    for (int i = 0; i < workers; i++) {
        struct msg_request end_request = { .start = -1, .end = -1 };
        if (mq_send(mq, (const char *)&end_request, sizeof(end_request), 0) == -1) {
            perror("mq_send failed");
            return EXIT_FAILURE;
        }
    }

    // Wait for all worker processes to finish
    for (int i = 0; i < workers; i++) {
        wait(NULL);
    }

    // Validate the result (although no unhandled error should occur)
    // The sum of the first n natural numbers is n * (n + 1) / 2
    long expected = n * (n + 1) / 2;
    if (sum_ptr->total != expected) {
        fprintf(stderr, "Error: expected %ld, got %ld\n", expected, sum_ptr->total);
        return EXIT_FAILURE;
    }

    // Display the final result
    printf("Result: %ld\n", sum_ptr->total);

    // Clean up resources
    munmap(sum_ptr, sizeof(struct global_sum));
    shm_unlink("/global_sum");
    mq_close(mq);
    mq_unlink("/sum_queue");
    sem_close(sem);
    sem_unlink("/sem");

    return EXIT_SUCCESS;
}
