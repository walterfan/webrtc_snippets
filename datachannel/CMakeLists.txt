cmake_minimum_required(VERSION 3.11)

project(dcsctp_demo)

if (APPLE)
  SET(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
  SET(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR})
  include_directories(${OPENSSL_ROOT_DIR}/include)
  link_directories(${OPENSSL_ROOT_DIR}/lib)
endif (APPLE)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(dcsctp_example dcsctp_example.cpp)
target_link_libraries(dcsctp_example ${OPENSSL_LIBRARIES})

install(TARGETS dcsctp_example RUNTIME DESTINATION bin)