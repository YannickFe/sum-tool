#    sum-tool - Dockerfile - Docker configuration to build and run the project in a container.
#
#    Copyright (C) 2025 Yannick Fenz'l
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.


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