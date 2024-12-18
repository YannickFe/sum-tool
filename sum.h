// Name: Yannick Fenz'l; Matrikelnummer: ***REMOVED***; Date: 07.12.2024
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
#define MAX_MSG_SIZE sizeof(struct msg_request)

#define TERMINATION_SIGNAL -1 // Macro for termination signal

#define PERM 0666 // Permissions for message queue, shared memory, and semaphore

#define MQ_NAME "/sum_queue" // Message queue name
#define SHM_NAME "/global_sum" // Shared memory name
#define SEM_NAME "/sem" // Semaphore name

// Message structure for passing requests
struct msg_request {
    long start; // Start of the range
    long end;   // End of the range
};

// Shared memory structure for global sum
struct global_sum {
    long total; // Total sum
};

#endif // SUM_H