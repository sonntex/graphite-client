cmake_minimum_required(VERSION 3.15)

project(variti-graphite)

set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

include_directories(include)

add_library(
  variti-graphite
  src/core/tc.cpp
  src/graphite/parameter.cpp
  src/graphite/client.cpp)
