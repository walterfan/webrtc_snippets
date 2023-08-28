cmake_minimum_required(VERSION 3.11)
project(zmq_test)

include_directories(${CMAKE_SOURCE_DIR}/inc)
link_directories(/usr/lib/x86_64-linux-gnu)
set(CMAKE_CXX_STANDARD 17)

FIND_PACKAGE(ZeroMQ REQUIRED)
 
if (APPLE)
  SET(XCODE_SDK_ROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
  include_directories(${XCODE_SDK_ROOT}/usr/include)
  link_directories(${XCODE_SDK_ROOT}/usr/lib)
endif (APPLE)

ADD_EXECUTABLE(zmq_server zmq_server.cpp)
ADD_EXECUTABLE(zmq_client zmq_client.cpp)

TARGET_LINK_LIBRARIES(zmq_client PRIVATE zmq)
TARGET_LINK_LIBRARIES(zmq_server PRIVATE zmq)
