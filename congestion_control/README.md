# Congestion control

1) import header files and library from webrtc library
2) do some testing for gcc remb and transport wide CC algorithms

# cmake example

```
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(.)
FIND_LIBRARY(COMM_LIB event "/usr/local/lib" NO_DEFAULT_PATH)
#/workspace/webrtc/webrtc-checkout/src/out/Default/obj/modules/congestion_controller
```

# practice

ln -s /workspace/webrtc/webrtc-checkout/src webrtc-src