
cmake_minimum_required(VERSION 3.10)

include(${ROOT_DIR}/cmake/External_tinyxml2.cmake)
include(${ROOT_DIR}/cmake/External_spdlog.cmake)
include(${ROOT_DIR}/cmake/External_cppformat.cmake)
include(${ROOT_DIR}/cmake/External_jemalloc.cmake)
include(${ROOT_DIR}/cmake/External_boost.cmake)

set(CisEngine_THIRDPARTYLIBS_ARGS "${CisEngine_THIRDPARTYLIBS_ARGS}"
  PARENT_SCOPE)

