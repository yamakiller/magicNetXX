if("${CMAKE_CURRENT_BINARY_DIR}" MATCHES " ")
  message(FATAL_ERROR "cannot use jemalloc configure with a space in the name of the build dir")
endif()

#set (JEMALLOC_LIB_PATH "/usr/local/lib/libjemalloc.so")

ExternalProject_Add(jemalloc
    DOWNLOAD_DIR ${download_dir}
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/jemalloc"
    URL "https://github.com/jemalloc/jemalloc/releases/download/5.2.0/jemalloc-5.2.0.tar.bz2"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-jemalloc-prefix=je_ --disable-valgrind
    CFLAGS=-std=gnu99\ -Wall\ -pipe\ -g3\ -O3\ -funroll-loops
    BUILD_COMMAND ${MAKE}
    INSTALL_DIR "${CisEngine_INSTALL_PREFIX}"
    INSTALL_COMMAND $(MAKE) install_lib_shared install_include
    BUILD_IN_SOURCE 1
    CMAKE_CACHE_ARGS
    ${CisEngine_DEFAULT_ARGS}
    ${CisEngine_THIRDPARTYLIBS_ARGS}
    )

set(JEMALLOC_ROOT "${CMAKE_CURRENT_BINARY_DIR}/jemalloc" CACHE INTERNAL "")
set(JEMALLOC_LIB "${BOOST_ROOT}/stage/lib" CACHE INTERNAL "")

list(APPEND CisEngine_THIRDPARTYLIBS_ARGS
    # Add Boost properties so correct version of Boost is found.
      "-DJEMALLOC_ROOT:PATH=${JEMALLOC_ROOT}"
      "-DJemalloc_INCLUDE_DIR:PATH=${JEMALLOC_ROOT}"
      "-DJemalloc_LIBRARIES:PATH=${JEMALLOC_LIB}")
    

#add_library(libjemalloc SHARED IMPORTED GLOBAL)
#set_target_properties(libjemalloc PROPERTIES IMPORTED_LOCATION ${JEMALLOC_LIB_PATH})
#add_dependencies(libjemalloc jemalloc)
#include_directories("${CMAKE_CURRENT_BINARY_DIR}/include/")

