cmake_minimum_required (VERSION 2.8)
project (certus_core)

file(GLOB h_files "*.h")
install(FILES ${h_files} DESTINATION include/certus)

add_subdirectory(version)
add_subdirectory(request)
