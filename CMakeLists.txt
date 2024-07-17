cmake_minimum_required(VERSION 3.26)
project(pagetables C)

set(CMAKE_C_STANDARD 11)

add_executable(pagetables main.c
        ../tlb/mlpt.h
        ../tlb/mlpt.c)
