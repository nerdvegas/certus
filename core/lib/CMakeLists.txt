cmake_minimum_required (VERSION 2.8)
project (certus_core)

file(GLOB h_files "*.h")
install(FILES ${h_files} DESTINATION include/certus)

add_subdirectory(util)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/util)
set(_CERTUS_LIBS ${_CERTUS_LIBS} certus_util )

add_subdirectory(version)

add_subdirectory(request)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/request)
set(_CERTUS_LIBS ${_CERTUS_LIBS} certus_request )

add_subdirectory(object)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/object)
set(_CERTUS_LIBS ${_CERTUS_LIBS} certus_object )

add_subdirectory(resolve)
