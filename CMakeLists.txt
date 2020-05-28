cmake_minimum_required(VERSION 3.16)
project(Maze)

set(CMAKE_CXX_STANDARD 14)
aux_source_directory(. source_list)
add_executable(Maze main.cpp  ${source_list})