cmake_minimum_required(VERSION 3.11)

project(audio_demo)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

add_executable(jitter_buffer_example jitter_buffer_example.cpp)
add_executable(audio_red_example audio_red_example.cpp)