
cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

include(${ROOT_DIR}/cmake/External_jemalloc.cmake)
include(${ROOT_DIR}/cmake/External_boost.cmake)

set(CisEngine_THIRDPARTYLIBS_ARGS "${CisEngine_THIRDPARTYLIBS_ARGS}"
  PARENT_SCOPE)

add_subdirectory("cppformat-master")
add_subdirectory("spdlog")
