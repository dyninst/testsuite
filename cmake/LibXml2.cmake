#========================================================================================================
# LibXml2.cmake
#
# Configure LibXml2 for Dyninst
#
#   ----------------------------------------
#
# Accepts the following CMake variables
#
# LibXml2_ROOT_DIR            - Hint directory that contains the LibXml2 installation
# LibXml2_INCLUDEDIR          - Hint directory that contains the LibXml2 headers files
#
# Exports the following CMake cache variables
#
# LibXml2_ROOT_DIR            - Computed base directory the of LibXml2 installation
# LibXml2_INCLUDE_DIRS        - LibXml2 include directories
# LibXml2_LIBRARY_DIRS        - Link directories for LibXml2 libraries
# LibXml2_DEFINITIONS         - Compile switches required for using LibXml2
# LibXml2_LIBRARIES           - LibXml2 library files
#
# NOTE:
# The exported LibXml2_ROOT_DIR can be different from the value provided by the user in the case that
# it is determined to build LibXml2 from source. In such a case, LibXml2_ROOT_DIR will contain the
# directory of the from-source installation.
#
#========================================================================================================

# -------------- PATHS --------------------------------------------------------

# Base directory the of LibXml2 installation
set(LibXml2_ROOT_DIR "/usr"
    CACHE PATH "Base directory the of LibXml2 installation")
    
# Hint directory that contains the LibXml2 header files
set(LibXml2_INCLUDEDIR "${LibXml2_ROOT_DIR}/include"
    CACHE PATH "Hint directory that contains the LibXml2 header files")

# -------------- PACKAGES -----------------------------------------------------

find_package(LibXml2)

# -------------- SOURCE BUILD -------------------------------------------------
if(LIBXML2_FOUND)
  set(_lx_root ${LibXml2_ROOT_DIR})
  set(_lx_inc_dir ${LIBXML2_INCLUDE_DIR})
  set(_lx_lib_dirs ${LIBXML2_LIBRARY})
  set(_lx_libs ${LIBXML2_LIBRARIES})
  add_library(LibXml2 SHARED IMPORTED)
else()
  message(STATUS "${LibXml2_ERROR_REASON}")
  message(STATUS "Attempting to build LibXml2 as external project")
  
  if(NOT UNIX)
    message(FATAL_ERROR "Building LibXml2 from source is not supported on this platform")
  endif()

  include(ExternalProject)
  ExternalProject_Add(
    LibXml2
    PREFIX ${CMAKE_BINARY_DIR}/LibXml2
    URL ftp://xmlsoft.org/libxml2/libxml2-2.9.9.tar.gz
    URL_MD5 c04a5a0a042eaa157e8e8c9eabe76bd6
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND
      <SOURCE_DIR>/configure
      	--prefix=${CMAKE_INSTALL_PREFIX} --disable-static --with-pic --without-python
    BUILD_COMMAND make
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND make install
  )

  set(_lx_root ${CMAKE_INSTALL_PREFIX})
  set(_lx_inc_dir ${_lx_root}/include/libxml2)
  set(_lx_lib_dirs ${_lx_root}/lib)
  set(_lx_libs ${_lx_lib_dirs}/libxml2.so)
endif()
  
# -------------- EXPORT VARIABLES ---------------------------------------------

set(LibXml2_ROOT_DIR ${_lx_root}
    CACHE PATH "Base directory the of LibXml2 installation"
    FORCE)
set(LibXml2_INCLUDE_DIRS ${_lx_inc_dir}
    CACHE PATH "LibXml2 header directory"
    FORCE)
set(LibXml2_LIBRARY_DIRS ${_lx_lib_dirs}
    CACHE PATH "LibXml2 library directory"
    FORCE)
set(LibXml2_LIBRARIES ${_lx_libs}
    CACHE FILEPATH "LibXml2 library files"
    FORCE)

message(STATUS "LibXml2 library dirs: ${LibXml2_LIBRARY_DIRS}")
message(STATUS "LibXml2 header dirs: ${LibXml2_INCLUDE_DIRS}")
message(STATUS "LibXml2 libraries: ${LibXml2_LIBRARIES}")
