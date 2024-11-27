// sum_worker.c
#include "sum.h"


int main() {
    // Open message queue and shared memory
    key_t key = ftok("sum.c", 'A');
    int msgid = msgget(key, 0666);

    int shm_fd = shm_open("/global_sum", O_RDWR, 0666);
    struct global_sum *sum_ptr = mmap(0, sizeof(struct global_sum), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    sem_t *sem = sem_open("/sem", O_RDWR);

    long total_requests = 0;

    while (1) {
        struct msg_request request;
        msgrcv(msgid, &request, sizeof(request) - sizeof(long), 1, 0);

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