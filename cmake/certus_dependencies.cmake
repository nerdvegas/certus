###########################################################################
#
# CMAKE project dependencies for Certus
#
###########################################################################
if (NOT CERTUS_SETUP_DEPS)

    # Define the version of Python we are using
    #
    find_package(PythonLibs)
    message(STATUS "Python found        ${PYTHONLIBS_FOUND}")
    message(STATUS "Python include dirs ${PYTHON_INCLUDE_DIRS}")
    message(STATUS "Python library      ${PYTHON_LIBRARIES}")

    include_directories ("${PYTHON_INCLUDE_DIRS}")
 
    # Define the version of boost we are using
    #
    set (Boost_USE_MULTITHREADED ON)
    set (Boost_ADDITIONAL_VERSIONS "1.45" "1.44" 
                                   "1.43" "1.43.0" "1.42" "1.42.0" 
                                   "1.41" "1.41.0" "1.40" "1.40.0"
                                   "1.39" "1.39.0" "1.38" "1.38.0"
                                   "1.37" "1.37.0" "1.34.1" "1_34_1")
    find_package (Boost 1.34 REQUIRED 
                  COMPONENTS python
                 )

    message(STATUS "Boost found        ${Boost_FOUND} ")
    message(STATUS "Boost version      ${Boost_VERSION}")
    message(STATUS "Boost include dirs ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost library dirs ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost libraries    ${Boost_LIBRARIES}")

    include_directories ("${Boost_INCLUDE_DIRS}")
    link_directories ("${Boost_LIBRARY_DIRS}")

endif()
