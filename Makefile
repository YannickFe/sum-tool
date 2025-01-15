CC = gcc
CFLAGS = -std=c11 -Wall -pthread

# Ziel-Programme
TARGETS = sum sum_worker

# Abhängigkeiten für jedes Zielprogramm
all: $(TARGETS)

sum: sum.c sum.h
	$(CC) $(CFLAGS) -o $@ sum.c

sum_worker: sum_worker.c
	$(CC) $(CFLAGS) -o $@ sum_worker.c

test_sum: test_sum.c
	$(CC) $(CFLAGS) -o $@ test_sum.c

docker:
	 docker build -t sum . &&  docker run -it --rm --name sum sum bash

# Clean-Up
clean:
	rm -f $(TARGETS)