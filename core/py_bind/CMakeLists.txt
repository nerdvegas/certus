cmake_minimum_required (VERSION 2.8)
project (certus_pybind)

include(GetPython)

message(STATUS "\nFinding python...")
GetPython(REQUIRED)
message(STATUS "Found python-${PYTHON_VERSION} at ${PYTHON_INCLUDE_DIRS}")

# boost already found but we need the python lib...
GetBoost(COMPONENTS python REQUIRED QUIET)

include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARY_DIRS})
set(_CERTUS_LIBS ${_CERTUS_LIBS} ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} )

install(FILES __init__.py DESTINATION python/certus)

include_directories(.)


add_subdirectory(util)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/util)
set(_CERTUS_LIBS ${_CERTUS_LIBS} certus_py_util )

add_subdirectory(version)
add_subdirectory(request)
