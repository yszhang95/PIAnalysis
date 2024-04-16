cmake_minimum_required(VERSION 3.1...3.27)

project(
  PIAnalysis
  VERSION 0.1
  LANGUAGES CXX)

set(USE_PIONEER_DOCKER FALSE CACHE BOOL "Build Libraries Under PIONEER Docker")

# shared library for reading data
if (NOT USE_PIONEER_DOCKER)
  add_subdirectory(shared)
else()
  set(PIONEER_LIB_SOURCE_DIR "$ENV{PIONEERSYS}/install")
  link_directories($ENV{PIONEERSYS}/install/lib)
endif()

add_subdirectory(acceptance)
