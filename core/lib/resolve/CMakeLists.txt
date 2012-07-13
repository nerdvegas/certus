cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_resolve)

file(GLOB cpp_files "*.cpp")
add_library(certus_resolve SHARED ${cpp_files})
target_link_libraries(certus_resolve ${_CERTUS_LIBS} )
install(TARGETS certus_resolve DESTINATION lib)

file(GLOB h_files "*.h")
install(FILES ${h_files} DESTINATION include/certus/resolve)
