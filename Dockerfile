# Dockerfile
FROM gcc:latest

RUN apt update && apt install -y gdb make cmake

# Set the working directory
WORKDIR /app

# Copy the source code and header file into the container
COPY *.c *.h  CMakeLists.txt ./

# Generate the Makefile using CMake
RUN cmake .

# Compile the C programs
RUN make