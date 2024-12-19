cmake_minimum_required(VERSION 3.22)
project(fluid)

set(CMAKE_CXX_STANDARD 20)

add_executable(fluid
        fluid.cpp
        simulation.h
        FastFixed.h)
