cmake_minimum_required(VERSION 3.31)
project(HundEvelisExtreme)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_executable(HundEvelisExtreme main.cpp)
#add_executable(HundEvelisExtreme ost.cpp)
#add_executable(HundEvelisExtreme test.cpp)
#add_executable(HundEvelisExtreme work.cpp)
target_link_libraries(HundEvelisExtreme ws2_32)
