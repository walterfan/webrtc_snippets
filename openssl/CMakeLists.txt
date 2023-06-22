cmake_minimum_required(VERSION 3.11)

project(openssl_demo)

if (APPLE)
  SET(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
  SET(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR})
  include_directories(${OPENSSL_ROOT_DIR}/include)
  link_directories(${OPENSSL_ROOT_DIR}/lib)
endif (APPLE)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(X509Certificate X509Certificate.cpp)
target_link_libraries(X509Certificate ${OPENSSL_LIBRARIES})

add_executable(dtls_example dtls_example.cpp)
target_link_libraries(dtls_example ${OPENSSL_LIBRARIES})

add_executable(simple_tls_server simple_tls_server.cpp)
target_link_libraries(simple_tls_server ${OPENSSL_LIBRARIES})

add_executable(tls_server tls_server.cpp)
target_link_libraries(tls_server ${OPENSSL_LIBRARIES})

add_executable(tls_client tls_client.cpp)
target_link_libraries(tls_client ${OPENSSL_LIBRARIES})

install(TARGETS X509Certificate RUNTIME DESTINATION bin)