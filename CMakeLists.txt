cmake_minimum_required(VERSION 3.1...3.27)

project(
  PIAnalysis
  VERSION 0.1
  LANGUAGES CXX)

# shared library for reading data
add_subdirectory(shared)

add_subdirectory(acceptance)
