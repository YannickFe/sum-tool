#    sum-tool - Makefile - Build instructions for the project.
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