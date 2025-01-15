# Sum Calculation Project

This project calculates the sum of the first `n` natural numbers using multiple worker processes. It uses POSIX message queues, shared memory, and semaphores for inter-process communication and synchronization.
The sum tool is implemented in C and can be built and run on any POSIX-compliant system.

It should be noted that the project is not optimized for performance, but rather for educational purposes. 
The goal is to demonstrate how to use POSIX IPC mechanisms to coordinate multiple processes and share resources. #
If you just want to calculate the sum of the first `n` natural numbers, you should use the Gauss formula `n * (n + 1) / 2` to get the result in constant time.

## Project Structure

- `sum.h`: Header file containing macro definitions and data structures.
- `sum.c`: Main program that initializes resources, forks worker processes, and calculates the sum.
- `sum_worker.c`: Worker program that processes tasks from the message queue and updates the global sum.
- `test_sum.c`: Test program to validate the sum calculation.
- `Makefile`: Build instructions for the project.
- `Dockerfile`: Docker configuration to build and run the project in a container.

## Prerequisites

- GCC
- Make
- Docker

## Building the Project

To build the project, run:

```sh
make
```

# Docker build

If you have Docker installed, you can build and run the project in a container for better portability:

```sh
make docker
```