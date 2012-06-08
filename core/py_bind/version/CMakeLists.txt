###########################################################################
#
# CMAKE project for Certus
#
###########################################################################
include_directories(${PROJECT_SOURCE_DIR}/pystring)
include_directories(${PROJECT_SOURCE_DIR}/version)

# Python bindings for the "version" library
#
file(GLOB cpp_files "*.cpp")
add_library(pyversion SHARED ${cpp_files})
target_link_libraries(pyversion version pystring ${Boost_LIBRARIES} ${PYTHON_LIBRARY})

set_target_properties(pyversion PROPERTIES PREFIX "")
set_target_properties(pyversion PROPERTIES OUTPUT_NAME "_version")

install(TARGETS pyversion DESTINATION ${CERTUS_INSTALL_PYTHON_DIR}/version/)

file(GLOB pyfiles "*.py")
install(FILES ${pyfiles} DESTINATION ${CERTUS_INSTALL_PYTHON_DIR}/version/)

