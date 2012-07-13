cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_pybind_resolve)


file(GLOB cpp_files "*.cpp")
add_library(resolve SHARED ${cpp_files})

target_link_libraries(resolve ${_CERTUS_LIBS} )
set_target_properties(resolve PROPERTIES PREFIX "")

install(TARGETS resolve DESTINATION python/certus)
