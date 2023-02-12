cmake_minimum_required(VERSION 3.22.0)

project(example LANGUAGES CXX)

add_subdirectory(tests)

add_library(example SHARED
  TcpClient.hpp
  TcpClient.cpp
  TcpConnection.hpp
  TcpConnection.cpp
  TcpServer.hpp
  TcpServer.cpp
  )

target_compile_features(example PRIVATE cxx_std_17)

target_compile_options(example PRIVATE -Wall -Wextra -Wpedantic -Werror -O0)

target_include_directories(example PUBLIC ..)

find_package(Boost 1.74.0 REQUIRED)
if (Boost_FOUND)
  target_include_directories(example PUBLIC ${Boost_INCLUDE_DIRS})  
endif()

