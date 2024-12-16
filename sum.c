// Name: Yannick Fenz'l; Matrikelnummer: ***REMOVED***; Date: 07.12.2024
// sum.c

#include "sum.h" // Include the header file
#include <sys/wait.h> // wait() to handle child processes
#include <mqueue.h>   // POSIX message queues
#include <fcntl.h>    // file control constants
#include <sys/mman.h> // shared memory
#include <semaphore.h> // semaphores
#include <stdio.h>    // standard I/O
#include <stdlib.h>   // standard library functions
#include <unistd.h>   // fork(), execlp(), and other POSIX functions
#include <errno.h> // errno descriptions
#include <limits.h> // LONG_MAX validation
#include <string.h> // strerror

#define WORKER_EXECUTABLE "./sum_worker"

long gauss_sum(long n) {
    return n * (n + 1) / 2;
}

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
        fprintf(stderr, "Invalid parameters. Ensure 0 < n, 1 <= chunksize <= %d, 1 <= workers <= %d.\n", MAX_CHUNKSIZE, MAX_WORKERS);
        return EXIT_FAILURE;
    }

    // Check for overflow in n or result
    if (n > LONG_MAX / 2 || gauss_sum(n) > LONG_MAX) {
        fprintf(stderr, "Overflow detected in calculating the sum for n = %ld. Choose a smaller value for n.\n", n);
        return EXIT_FAILURE;
    }

    // Initialize the message queue
    struct mq_attr attr;
    attr.mq_flags = 0; // Blocking mode
    attr.mq_maxmsg = 10; // Maximum number of messages
    attr.mq_msgsize = MAX_MSG_SIZE; // Maximum message size
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t)-1) {
        fprintf(stderr, "mq_open failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // Validate message queue limits dynamically
    struct mq_attr actual_attr;
    if (mq_getattr(mq, &actual_attr) == -1) {
        fprintf(stderr, "mq_getattr failed: %s\n", strerror(errno));
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }

    if (actual_attr.mq_maxmsg < attr.mq_maxmsg) {
        fprintf(stderr, "System message queue limit (%ld) is lower than requested (%ld).\n", actual_attr.mq_maxmsg, attr.mq_maxmsg);
    }

    // Create shared memory for storing the global sum
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        fprintf(stderr, "shm_open failed: %s\n", strerror(errno));
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }

    // Set the size of shared memory
    if (ftruncate(shm_fd, sizeof(struct global_sum)) == -1) {
        fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
        shm_unlink(SHM_NAME);
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }

    // Map shared memory to the process's address space
    struct global_sum *sum_ptr = mmap(0, sizeof(struct global_sum), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sum_ptr == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        shm_unlink(SHM_NAME);
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }
    sum_ptr->total = 0; // Initialize the global sum to 0

    // Create a semaphore for synchronization
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        fprintf(stderr, "sem_open failed: %s\n", strerror(errno));
        munmap(sum_ptr, sizeof(struct global_sum));
        shm_unlink(SHM_NAME);
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }

    // Fork worker processes
    for (int i = 0; i < workers; i++) {
        if (fork() == 0) {
            // Replace the child process with the worker executable
            execlp(WORKER_EXECUTABLE, WORKER_EXECUTABLE, NULL);
            fprintf(stderr, "execlp failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Divide the range into chunks and send tasks to the workers
    for (long start = 1; start <= n; start += chunksize) {
        long end = (start + chunksize - 1 > n) ? n : start + chunksize - 1;

        struct msg_request request = {
            .start = start,
            .end = end
        };
        if (mq_send(mq, (const char *)&request, sizeof(request), 0) == -1) {
            fprintf(stderr, "mq_send failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }

    // Send termination signals to all workers
    for (int i = 0; i < workers; i++) {
        struct msg_request end_request = {
            .start = TERMINATION_SIGNAL,
            .end = TERMINATION_SIGNAL
        };
        if (mq_send(mq, (const char *)&end_request, sizeof(end_request), 0) == -1) {
            fprintf(stderr, "mq_send failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }

    // Wait for all worker processes to finish
    for (int i = 0; i < workers; i++) {
        wait(NULL);
    }

    // Validate the result (although no unhandled error should occur)
    // The sum of the first n natural numbers is n * (n + 1) / 2
    long expected = gauss_sum(n);
    if (sum_ptr->total != expected) {
        fprintf(stderr, "Error: expected %ld, got %ld\n", expected, sum_ptr->total);
        return EXIT_FAILURE;
    }

    // Display the final result
    printf("Result: %ld\n", sum_ptr->total);

    // Clean up resources
    munmap(sum_ptr, sizeof(struct global_sum));
    shm_unlink(SHM_NAME);
    mq_close(mq);
    mq_unlink(MQ_NAME);
    sem_close(sem);
    sem_unlink(SEM_NAME);

    return EXIT_SUCCESS;
}
