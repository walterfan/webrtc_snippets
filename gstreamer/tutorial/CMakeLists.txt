cmake_minimum_required(VERSION 3.11)
project(gstreamer_test)

set(CMAKE_CXX_STANDARD 20)

#include(FindPkgConfig)# equals `pkg-config --cflags --libs gstreamer-1.0`
#pkg_check_modules(GST   REQUIRED gstreamer-1.0)
#pkg_check_modules(GSTAPP REQUIRED gstreamer-app-1.0)

set(GST_INCLUDE_DIRS /Library/Frameworks/GStreamer.framework/Headers)
set(GST_LIBRARY_DIRS /Library/Frameworks/GStreamer.framework/Versions/Current/lib)

include_directories(${GST_INCLUDE_DIRS})

link_directories(${GST_LIBRARY_DIRS})

add_executable(basic-tutorial-1 basic-tutorial-1.c)


target_link_libraries(basic-tutorial-1 gstreamer-1.0 )
