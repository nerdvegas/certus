###########################################################################
#
# CMAKE project for Certus
#
###########################################################################

# The pystring library
#
set(pystring_h pystring.h)
set(pystring_cpp pystring.cpp)

add_library(pystring STATIC ${pystring_cpp})

install(TARGETS pystring DESTINATION ${CERTUS_INSTALL_LIB_DIR})
install(FILES ${pystring_h} DESTINATION ${CERTUS_INSTALL_INCLUDE_DIR})

