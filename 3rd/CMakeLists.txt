
cmake_minimum_required(VERSION 3.10)

INCLUDE(ExternalProject)

set(PROJ_NAME deps)

if (APPLE)
set (JEMALLOC_LIB_PATH "/usr/local/lib/libjemalloc.dylib")
else ()
set (JEMALLOC_LIB_PATH "/usr/local/lib/libjemalloc.so")
endif()


ExternalProject_Add(jemalloc
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jemalloc
    URL "https://github.com/jemalloc/jemalloc/releases/download/5.2.0/jemalloc-5.2.0.tar.bz2"
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/jemalloc/configure --with-jemalloc-prefix=je_ --disable-valgrind
    CFLAGS=-std=gnu99\ -Wall\ -pipe\ -g3\ -O3\ -funroll-loops
    BUILD_COMMAND ${MAKE}
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/
    INSTALL_COMMAND $(MAKE) install_lib_shared install_include)
add_library(libjemalloc SHARED IMPORTED GLOBAL)
set_target_properties(libjemalloc PROPERTIES IMPORTED_LOCATION ${JEMALLOC_LIB_PATH})
add_dependencies(libjemalloc jemalloc)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include/")

find_package(Boost COMPONENTS system coroutine context)
if (NOT Boost_FOUND)
  ExternalProject_Add(boost
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boost
    URL "https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2"
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/boost/bootstrap.sh
    --with-libraries=system
    --with-libraries=coroutine
    --with-libraries=context
    BUILD_COMMAND  ./b2 install
    BUILD_IN_SOURCE 
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/
    INSTALL_COMMAND "")
endif()
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include/")



add_subdirectory("cppformat-master")
add_subdirectory("spdlog")