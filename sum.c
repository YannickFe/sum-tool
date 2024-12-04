// sum.c
#include "sum.h"
#include <sys/wait.h> // include wait.h for wait() function
#include <mqueue.h>   // include mqueue.h for POSIX message queues

#define MAX_MSG_SIZE sizeof(struct msg_request) // Define maximum message size

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s n chunksize workers\n", argv[0]);
        return EXIT_FAILURE;
    }

    long n = atol(argv[1]);
    long chunksize = atol(argv[2]);
    int workers = atoi(argv[3]);

    if (n <= 0 || chunksize < 1 || chunksize > MAX_CHUNKSIZE || workers < 1 || workers > MAX_WORKERS) {
        fprintf(stderr, "Invalid parameters.\n");
        return EXIT_FAILURE;
    }

    // Create POSIX message queue with attributes
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Maximum number of messages in the queue
    attr.mq_msgsize = MAX_MSG_SIZE; // Maximum message size
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open("/sum_queue", O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return EXIT_FAILURE;
    }

    // Create shared memory for global sum
    int shm_fd = shm_open("/global_sum", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(struct global_sum));
    struct global_sum *sum_ptr = mmap(0, sizeof(struct global_sum), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    sum_ptr->total = 0;

    // Create semaphore for synchronization
    sem_t *sem = sem_open("/sem", O_CREAT, 0666, 1);

    // Fork worker processes
    for (int i = 0; i < workers; i++) {
        if (fork() == 0) {
            execlp("./sum_worker", "sum_worker", NULL);
            perror("execlp failed");
            exit(EXIT_FAILURE);
        }
    }

    // Send tasks to workers via message queue
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

    // Send termination messages to workers
    for (int i = 0; i < workers; i++) {
        struct msg_request end_request;
        end_request.start = -1; // Indicate termination
        end_request.end = -1;
        if (mq_send(mq, (const char *)&end_request, sizeof(end_request), 0) == -1) {
            perror("mq_send failed");
            return EXIT_FAILURE;
        }
    }

    // Wait for all children to finish
    for (int i = 0; i < workers; i++) {
        wait(NULL);
    }

    printf("Result: %ld\n", sum_ptr->total);

    // Cleanup
    munmap(sum_ptr, sizeof(struct global_sum));
    shm_unlink("/global_sum");
    mq_close(mq);
    mq_unlink("/sum_queue");
    sem_close(sem);
    sem_unlink("/sem");

    return EXIT_SUCCESS;
}