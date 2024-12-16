# Dockerfile
FROM gcc:latest

RUN apt update && apt install -y gdb make cmake

# Set the working directory
WORKDIR /app

# Copy the source code and header file into the container
COPY *.c *.h  Makefile ./

# Compile the C programs
RUN make

RUN make test_sum
RUN ./test_sum