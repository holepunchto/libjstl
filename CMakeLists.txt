cmake_minimum_required(VERSION 4.0)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-prelude REQUIRED PATHS node_modules/cmake-prelude)
find_package(cmake-test REQUIRED PATHS node_modules/cmake-test)

project(jstl LANGUAGES C CXX VERSION 0.0.0)

add_library_targets(
  jstl
  HEADERS
    include/jstl.h
  FEATURES
    cxx_std_20
  SOURCES NONE
)

add_test_subdirectory(test)
