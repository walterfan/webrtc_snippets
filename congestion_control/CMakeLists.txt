cmake_minimum_required(VERSION 3.11)
project(rtc_cc_test)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

set(CMAKE_CXX_STANDARD 17)
include_directories(inc)

add_executable(waltertest 
    src/waltertest.cpp
    inc/detector.h
    inc/estimator.h
)

target_link_libraries(waltertest PkgConfig::deps)
