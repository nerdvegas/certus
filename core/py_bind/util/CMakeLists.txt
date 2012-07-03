cmake_minimum_required (VERSION 2.8)
project (certus_py_util)

add_definitions(-fPIC)
add_library(certus_py_util STATIC util.cpp)
