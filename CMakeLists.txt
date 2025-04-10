cmake_minimum_required(VERSION 3.29)
project(Paradigms3)

set(CMAKE_CXX_STANDARD 20)

add_executable(Paradigms3 main.cpp
        request.h
        device.h
        queueing.h)
