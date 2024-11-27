// sum.h
#ifndef SUM_H
#define SUM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_WORKERS 9
#define MAX_CHUNKSIZE 9999

// Message structure for passing requests
struct msg_request {
    long mtype; // Message type
    long start; // Start of the range
    long end;   // End of the range
};

// Shared memory structure for global sum
struct global_sum {
    long total; // Total sum
};

#endif // SUM_H