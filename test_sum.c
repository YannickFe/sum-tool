/*
* sum-tool - test_sum.c - Test program to validate the sum calculation.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// This is a very basic test implementation for the sum program

long calculate_expected_sum(long n) {
    return (n * (n + 1)) / 2;
}

void run_test(long n, long chunksize, int workers) {
    // Prepare command to run the sum program
    char command[256];
    snprintf(command, sizeof(command), "./sum %ld %ld %d", n, chunksize, workers);

    // Execute the sum program
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run sum");
        exit(EXIT_FAILURE);
    }

    // Read output from the sum program
    char result_line[128];
    long actual_result = 0;
    while (fgets(result_line, sizeof(result_line), fp) != NULL) {
        if (sscanf(result_line, "Result: %ld", &actual_result) == 1) {
            break; // Extract actual result
        }
    }

    // Close the process
    pclose(fp);

    // Calculate expected result
    long expected_result = calculate_expected_sum(n);

    // Validate results
    if (actual_result == expected_result) {
        printf("Test passed! Expected: %ld, Actual: %ld\n", expected_result, actual_result);
    } else {
        printf("Test failed! Expected: %ld, Actual: %ld\n", expected_result, actual_result);
    }
}

int main() {
    // Define test cases
    long test_cases[][3] = {
        {1000000, 100, 3},
        {50, 15, 2},
        {10000, 1000, 4},
        {10000000, 1000, 5},
        {5000, 500, 2}
    };

    // Run tests
    for (int i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        run_test(test_cases[i][0], test_cases[i][1], test_cases[i][2]);
    }

    return EXIT_SUCCESS;
}