cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_pybind_version)


file(GLOB cpp_files "*.cpp")
add_library(version SHARED ${cpp_files})

target_link_libraries(version ${_CERTUS_LIBS})
set_target_properties(version PROPERTIES PREFIX "")

install(TARGETS version DESTINATION python/certus)
