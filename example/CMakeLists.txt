cmake_minimum_required(VERSION 3.15)

project(variti-graphite-examples)

set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_LOG_USE_NATIVE_SYSLOG)

include_directories(../include)

add_executable(
  graphite-example1
  ../src/core/tc.cpp
  ../src/graphite/parameter.cpp
  ../src/graphite/client.cpp
  graphite_example1.cpp)
target_link_libraries(
  graphite-example1
  boost_log
  pthread)
