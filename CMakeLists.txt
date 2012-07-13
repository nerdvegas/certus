cmake_minimum_required (VERSION 2.8)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
project (certus)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(GetBoost)

set(BOOST_ROOT $ENV{BOOST_ROOT})

####
#### USER EDIT BEGIN HERE...
####
# Set paths to other software here
#set(BOOST_ROOT /wherever/your/boost/is/installed)
#set(PYTHON_ROOT /wherever/your/python/is/installed)

# Set options to ON/OFF here
option(CERTUS_STATIC_LINK_BOOST "Statically link to boost libs" OFF)
option(CERTUS_BUILD_PY_BINDINGS "Build python bindings" ON)
option(CERTUS_BUILD_TEST "Build unit tests" ON)
####
#### USER EDIT END HERE.
####


set(CERTUS_VERSION_MAJOR 0)
set(CERTUS_VERSION_MINOR 0)
set(CERTUS_VERSION_PATCH 0)
set(CERTUS_VERSION ${CERTUS_VERSION_MAJOR}.${CERTUS_VERSION_MINOR}.${CERTUS_VERSION_PATCH})
message(STATUS "Building certus-${CERTUS_VERSION}...")


# Find Dependencies
###############################################################################

message(STATUS "\nFinding boost...")
if(CERTUS_STATIC_LINK_BOOST)
    set(Boost_USE_STATIC_LIBS ON)
endif()
GetBoost(REQUIRED)
message(STATUS "Found boost-${Boost_NORMALIZED_VERSION} at ${Boost_LIBRARY_DIRS}")


# Include Dependencies
###############################################################################

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


# Build Subdirs
###############################################################################

add_subdirectory(contrib/pystring)
include_directories(contrib)
include_directories(contrib/pystring)
link_directories(${CMAKE_BINARY_DIR}/contrib/pystring)

set(_CERTUS_LIBS certus_pystring)

include_directories(core/lib)
add_subdirectory(core/lib)

if(CERTUS_BUILD_PY_BINDINGS)
    add_subdirectory(core/py_bind)
endif()




