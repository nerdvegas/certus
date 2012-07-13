cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_object)

# boost already found but we need the filesystem lib...
GetBoost(COMPONENTS filesystem REQUIRED QUIET)
set(_CERTUS_LIBS ${_CERTUS_LIBS} ${Boost_LIBRARIES} )

file(GLOB cpp_files "*.cpp")
add_library(certus_object SHARED ${cpp_files})
target_link_libraries(certus_object ${_CERTUS_LIBS} )
install(TARGETS certus_object DESTINATION lib)

file(GLOB h_files "*.h")
install(FILES ${h_files} DESTINATION include/certus/object)
