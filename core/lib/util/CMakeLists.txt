cmake_minimum_required (VERSION 2.8)
project (certus_util)

add_definitions(-fPIC)
file(GLOB cpp_files "*.cpp")
add_library(certus_util STATIC ${cpp_files})
