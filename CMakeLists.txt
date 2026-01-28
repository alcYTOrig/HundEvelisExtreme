cmake_minimum_required(VERSION 3.20)
project(HundEvelisExtreme)

set(CMAKE_CXX_STANDARD 17)

add_executable(HundEvelisExtreme main.cpp)

# Для Windows
if(WIN32)
    target_compile_definitions(HundEvelisExtreme PRIVATE _WIN32_WINNT=0x0600)
endif()

# Статическая линковка для MinGW
if(MINGW)
    target_link_options(HundEvelisExtreme PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
    )
endif()

target_link_libraries(HundEvelisExtreme ws2_32)