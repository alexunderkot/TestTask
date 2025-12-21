cmake_minimum_required(VERSION 3.10)
project(TestTask)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для Windows
if(WIN32)
    add_executable(TestTask TestTask.cpp)
    target_link_libraries(TestTask ws2_32)
else()
    # Для Linux/Unix
    add_executable(TestTask TestTask.cpp)
    target_link_libraries(TestTask pthread)
endif()