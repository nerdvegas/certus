###########################################################################
#
# CMAKE project for Certus
#
###########################################################################
include_directories(${PROJECT_SOURCE_DIR}/pystring)

# The "version" library
#
file(GLOB version_h "*.h")
set(version_cpp version.cpp)
add_library(version STATIC ${version_cpp})
target_link_libraries(version pystring)

install(TARGETS version DESTINATION ${CERTUS_INSTALL_LIB_DIR})
install(FILES ${version_h} DESTINATION ${CERTUS_INSTALL_INCLUDE_DIR})

# Simple test program
#
set(version_test_cpp test.cpp)
add_executable(test ${version_test_cpp})
target_link_libraries(test pystring version)

install(TARGETS test DESTINATION ${CERTUS_INSTALL_BIN_DIR})

