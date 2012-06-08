###########################################################################
#
# CMAKE project for Certus
#
###########################################################################
project (certus)

# Set to 1 for super-verbose
#
SET(CMAKE_VERBOSE_MAKEFILE 1)

# Cerus version
#
set (CERTUS_VERSION_MAJOR 0)
set (CERTUS_VERSION_MINOR 0)
set (CERTUS_VERSION_PATCH 0)

# CMake requirements
#
cmake_minimum_required (VERSION 2.6)
set (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

# Don't allow in-source builds (The root Makefile
# should prevent this from happening)
#
if (${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message (FATAL_ERROR "Not allowed to run in-source build!")
endif ()

if (NOT CMAKE_BUILD_TYPE) 
    set (CMAKE_BUILD_TYPE "Release") 
endif ()

# Additional cmake files
#
set(CERTUS_CMAKE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH_ORIG  ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_ORIG} ${CERTUS_CMAKE_PATH})

# Find all external requirements
#
set(BOOST_ROOT ${BOOST_ROOT})
include(certus_dependencies)

# Flags
#
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions ("-DDEBUG=1")
endif ()
if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# Install Locations
#
set(CERTUS_INSTALL_BIN_DIR       "${CMAKE_INSTALL_PREFIX}/bin")
set(CERTUS_INSTALL_LIB_DIR       "${CMAKE_INSTALL_PREFIX}/lib")
set(CERTUS_INSTALL_INCLUDE_DIR   "${CMAKE_INSTALL_PREFIX}/include")
set(CERTUS_INSTALL_PYTHON_DIR    "${CMAKE_INSTALL_PREFIX}/python")
message(STATUS "Install directory  ${CMAKE_INSTALL_PREFIX}")

###########################################################################

# Pystring lib
#
add_subdirectory(pystring)

# Version library
#
add_subdirectory(version)

# Python bindings
if (Boost_PYTHON_FOUND)
    add_subdirectory(python)
endif ()

