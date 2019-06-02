if("${CMAKE_CURRENT_BINARY_DIR}" MATCHES " ")
  message(FATAL_ERROR "cannot use jemalloc configure with a space in the name of the build dir")
endif()


ExternalProject_Add(jemalloc
    DOWNLOAD_DIR ${download_dir}
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/jemalloc"
    URL "https://github.com/jemalloc/jemalloc/releases/download/5.2.0/jemalloc-5.2.0.tar.bz2"
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-jemalloc-prefix=je_ --disable-valgrind
    CFLAGS=-std=gnu99\ -Wall\ -pipe\ -g3\ -O3\ -funroll-loops
    BUILD_COMMAND ${MAKE}
    INSTALL_DIR "${CisEngine_INSTALL_PREFIX}"
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
    CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    )

ExternalProject_Get_Property(jemalloc install_dir)
set(JEMALLOC_ROOT "${CMAKE_CURRENT_BINARY_DIR}/jemalloc" CACHE INTERNAL "")
set(JEMALLOC_LIB "${JEMALLOC_ROOT}/lib" CACHE INTERNAL "")

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add Boost properties so correct version of Boost is found.
      "-DJEMALLOC_ROOT:PATH=${JEMALLOC_ROOT}"
      "-DJemalloc_INCLUDE_DIR:PATH=${JEMALLOC_ROOT}"
      "-DJemalloc_LIBRARIES:PATH=${JEMALLOC_LIB}")

add_custom_command(TARGET jemalloc POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${JEMALLOC_LIB} ${CisEngine_INSTALL_PREFIX}/lib)
    

