cmake_minimum_required(VERSION 3.26)
project(tvparser)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS json)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_libraries(${Boost_LIBRARIES})
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(OpenSSL REQUIRED)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

add_executable(tvparser
  src/tvparser.cpp
  src/tvparser.hpp
  src/main.cpp
)

target_include_directories(tvparser PRIVATE ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(tvparser PRIVATE ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} Boost::json)
