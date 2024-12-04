// sum_worker.c
#include "sum.h"
#include <mqueue.h> // include mqueue.h for POSIX message queues

int main() {
    // Open POSIX message queue and shared memory
    mqd_t mq = mq_open("/sum_queue", O_RDWR);
    if (mq == (mqd_t)-1) {
        perror("mq_open failed");
        return EXIT_FAILURE;
    }

    int shm_fd = shm_open("/global_sum", O_RDWR, 0666);
    struct global_sum *sum_ptr = mmap(0, sizeof(struct global_sum), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *sem = sem_open("/sem", O_RDWR);

    long total_requests = 0;

    while (1) {
        struct msg_request request;
        if (mq_receive(mq, (char *)&request, sizeof(request), NULL) == -1) {
            perror("mq_receive failed");
            return EXIT_FAILURE;
        }

        if (request.start == -1) { // Check for termination signal
            break;
        }

        long partial_sum = 0;
        for (long i = request.start; i <= request.end; i++) {
            partial_sum += i;
        }

        sem_wait(sem); // Lock before updating shared memory
        sum_ptr->total += partial_sum;
        sem_post(sem); // Unlock after updating

        total_requests++;
    }

    printf("pid %d done after %ld requests\n", getpid(), total_requests);

    munmap(sum_ptr, sizeof(struct global_sum));

    return EXIT_SUCCESS;
}