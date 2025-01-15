/*
* sum-tool - sum.h - Header file containing macro definitions and data structures for the sum program.
 *
 * Copyright (C) 2025 Yannick Fenz'l
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


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