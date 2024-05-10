cmake_minimum_required(VERSION 3.18)

project(pipe_research)

set(CMAKE_CXX_STANDARD 20)
include_directories(../)
include_directories(../import)

add_executable(test test.cc)
