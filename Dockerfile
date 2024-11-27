# Dockerfile
FROM gcc:latest

RUN apt update && apt install -y gdb

# Set the working directory
WORKDIR /app

# Copy the source code and header file into the container
COPY sum.c sum_worker.c sum.h ./

# Compile the C programs
RUN gcc -g -o sum sum.c -lpthread -lrt && gcc -g -o sum_worker sum_worker.c -lpthread -lrt