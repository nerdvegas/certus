cmake_minimum_required(VERSION 2.8)
project(certus_pystring)

add_definitions(-fPIC)
add_library(certus_pystring STATIC pystring.cpp)
