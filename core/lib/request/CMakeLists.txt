cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_request)

file(GLOB cpp_files "*.cpp")
add_library(certus_request SHARED ${cpp_files})
target_link_libraries(certus_request ${_CERTUS_LIBS} )
install(TARGETS certus_request DESTINATION lib)

file(GLOB h_files "*.h")
install(FILES ${h_files} DESTINATION include/certus/request)
