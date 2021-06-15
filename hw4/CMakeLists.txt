cmake_minimum_required(VERSION 3.12)
project(CA_hw4)

set(CMAKE_CXX_STANDARD 11)

add_executable(CA_hw4 main.c core_api.cpp core_api.h sim_api.c sim_api.h)
