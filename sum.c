// sum.c
#include "sum.h"
#include <sys/wait.h> // include wait.h for wait() function

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

    // Create message queue
    key_t key = ftok("sum.c", 'A');
    int msgid = msgget(key, IPC_CREAT | 0666);

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
        request.mtype = 1;
        request.start = start;
        request.end = end;
        msgsnd(msgid, &request, sizeof(request) - sizeof(long), 0);
    }

    // Send termination messages to workers
    for (int i = 0; i < workers; i++) {
        struct msg_request end_request;
        end_request.mtype = 1;
        end_request.start = -1; // Indicate termination
        msgsnd(msgid, &end_request, sizeof(end_request) - sizeof(long), 0);
    }

    // Wait for all children to finish
    for (int i = 0; i < workers; i++) {
        wait(NULL);
    }

    printf("Result: %ld\n", sum_ptr->total);

    // Cleanup
    munmap(sum_ptr, sizeof(struct global_sum));
    shm_unlink("/global_sum");
    msgctl(msgid, IPC_RMID, NULL);
    sem_close(sem);
    sem_unlink("/sem");

    return EXIT_SUCCESS;
}