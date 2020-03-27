cmake_minimum_required(VERSION 3.14)
project(OsuBot)

set(CMAKE_CXX_STANDARD 14)

add_executable(OsuBot main.cpp memtools.h memtools.cpp OsuProcess.cpp OsuProcess.h)