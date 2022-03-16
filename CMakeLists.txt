cmake_minimum_required(VERSION 3.0.0)
project(webrtc_primer VERSION 0.1.0)

include(CTest)
enable_testing()

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")

message("system: ", ${CMAKE_SYSTEM_NAME})

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_definitions(-DWEBRTC_WIN)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX)
endif()
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall  -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")
set(CMAKE_CXX_FLAGS "-fPIC")  
set(CMAKE_C_FLAGS "-fPIC") 
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-DNDEBUG)
add_definitions(-std=c++11  )#-fno-rtti
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(media)
add_subdirectory(signal_server)
add_subdirectory(webrtc_cc)

add_executable(webrtc_primer webrtc_primer.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
