#~----------------------------------------------------------------------------~#
# SKA package configuration
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# If a C++14 compiler is available, then set the appropriate flags
#------------------------------------------------------------------------------#

include(cxx14)

check_for_cxx14_compiler(CXX14_COMPILER)

if(CXX14_COMPILER)
  enable_cxx14()
else()
  message(FATAL_ERROR "C++14 compatible compiler not found")
endif()

#------------------------------------------------------------------------------#
# LLVM
#------------------------------------------------------------------------------#

find_package(LLVM 3.6.1 REQUIRED all)

#------------------------------------------------------------------------------#
# Xerces
#------------------------------------------------------------------------------#

find_package(XercesC REQUIRED)
find_package(YamlCpp REQUIRED)

#------------------------------------------------------------------------------#
# Graphviz
#------------------------------------------------------------------------------#

find_package(Graphviz REQUIRED)

#------------------------------------------------------------------------------#
# Qt
#------------------------------------------------------------------------------#

find_package(Qt4 REQUIRED)

#------------------------------------------------------------------------------#
# Qwt
#------------------------------------------------------------------------------#

find_package(Qwt REQUIRED)

#~---------------------------------------------------------------------------~-#
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
