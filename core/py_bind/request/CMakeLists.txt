cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_pybind_request)


file(GLOB cpp_files "*.cpp")
add_library(request SHARED ${cpp_files})

target_link_libraries(request ${_CERTUS_LIBS} )
set_target_properties(request PROPERTIES PREFIX "")

install(TARGETS request DESTINATION python/certus)
