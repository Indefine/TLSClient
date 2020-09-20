cmake_minimum_required (VERSION 3.5.1)
project(SasClient)

set( CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")

find_package(OpenSSL)
link_libraries(${OPENSSL_LIBRARIES})

add_subdirectory(client)
add_subdirectory(server)
