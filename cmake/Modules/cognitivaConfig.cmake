INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_COGNITIVA cognitiva)

FIND_PATH(
    COGNITIVA_INCLUDE_DIRS
    NAMES cognitiva/api.h
    HINTS $ENV{COGNITIVA_DIR}/include
        ${PC_COGNITIVA_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    COGNITIVA_LIBRARIES
    NAMES gnuradio-cognitiva
    HINTS $ENV{COGNITIVA_DIR}/lib
        ${PC_COGNITIVA_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(COGNITIVA DEFAULT_MSG COGNITIVA_LIBRARIES COGNITIVA_INCLUDE_DIRS)
MARK_AS_ADVANCED(COGNITIVA_LIBRARIES COGNITIVA_INCLUDE_DIRS)

