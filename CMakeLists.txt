cmake_minimum_required(VERSION 3.31)
project(HundEvelisExtreme)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable(HundEvelisExtreme main.cpp)
#add_executable(HundEvelisExtreme test.cpp)
