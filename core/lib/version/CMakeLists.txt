cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus_version)


file(GLOB h_files "*.h")
install(FILES ${h_files} DESTINATION include/certus/version)

if(CERTUS_BUILD_TEST)
    add_executable(version_test test.cpp)
    target_link_libraries(version_test ${_CERTUS_LIBS})
endif()
