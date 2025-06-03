cmake_minimum_required(VERSION 3.31)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

project(jstl CXX)

add_library(jstl INTERFACE)

set_target_properties(
  jstl
  PROPERTIES
  C_STANDARD 11
  CXX_STANDARD 20
  CXX_SCAN_FOR_MODULES OFF
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  jstl
  INTERFACE
    include/jstl.h
)

target_include_directories(
  jstl
  INTERFACE
    include
)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
