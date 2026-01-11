cmake_minimum_required(VERSION 3.20)
project(HundEvelisExtreme)

set(CMAKE_CXX_STANDARD 23)

# Разрешить множественные определения
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition")

add_executable(HundEvelisExtreme main.cpp)

# Сначала статические библиотеки
target_link_options(HundEvelisExtreme PRIVATE
        "-static-libstdc++"
        "-static-libgcc"
)

# Затем системные библиотеки (каждая по одному разу!)
target_link_libraries(HundEvelisExtreme
        ws2_32
)