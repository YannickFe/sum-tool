cmake_minimum_required(VERSION 3.30)
project(PA3 C)

set(CMAKE_C_STANDARD 11)

add_executable(sum sum.c)
add_executable(sum_worker sum_worker.c)


target_link_libraries(sum pthread)
target_link_libraries(sum_worker pthread)