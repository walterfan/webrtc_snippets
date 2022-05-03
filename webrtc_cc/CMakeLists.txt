cmake_minimum_required(VERSION 3.11)
project(rtc_cc_test)

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

find_library(CORE_FOUNDATION Foundation)
find_library(APPLICATION_SERVICES ApplicationServices)
find_library(CORE_SERVICES CoreServices)

set(EXTRA_LIBS ${CORE_FOUNDATION} ${APPLICATION_SERVICES} ${CORE_SERVICES})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT "$ENV{WEBRTC_SRC}" STREQUAL "")
    set(WEBRTC_SRC "$ENV{WEBRTC_SRC}" CACHE INTERNAL "Copied from environment variable")
    set(ABSL_SRC   "${WEBRTC_SRC}/third_party/abseil-cpp" CACHE INTERNAL "set absl include folder")
    set(WEBRTC_LIB "${WEBRTC_SRC}/out/Default/obj" CACHE INTERNAL "set webrtc library folder")
endif()

message("WEBRTC Source Folder: ${WEBRTC_SRC}")
message("WEBRTC Library Folder: ${WEBRTC_LIB}")
message("EXTRA_LIBS: ${EXTRA_LIBS}")

include_directories(inc /usr/local/include ${WEBRTC_SRC})
link_directories(/usr/local/lib /usr/lib ${WEBRTC_LIB})

#gn gen out/x64/Debug --args='is_debug=true target_os="mac" is_component_build=false rtc_include_tests=false target_cpu="x64"'
#gn gen out/arm64 --args='target_os="android" target_cpu="arm64" use_custom_libcxx=false'
#gn gen out/x64/Debug --args='is_debug=true target_os="mac" is_component_build=false rtc_include_tests=false target_cpu="x64" use_custom_libcxx=false'

add_executable(cctest 
    src/waltertest.cpp
    inc/detector.h
    inc/estimator.h
)
add_executable(estimator_test 
    test/estimator_test.cpp
    test/unit_test.cpp
    inc/detector.h
    inc/estimator.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_link_libraries(cctest PkgConfig::deps)
target_link_libraries(estimator_test libgtest.a libgmock.a libwebrtc.a ${EXTRA_LIBS})