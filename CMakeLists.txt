cmake_minimum_required(VERSION 3.25)
project(PA3 C)

set(CMAKE_C_STANDARD 11)

add_executable(sum sum.c sum.h)
add_executable(sum_worker sum_worker.c)
add_executable(test_sum test_sum.c)


target_link_libraries(sum pthread)
target_link_libraries(sum_worker pthread)