set(base_source "${CMAKE_CURRENT_SOURCE_DIR}/base")
set(base_build "${CMAKE_CURRENT_BINARY_DIR}/base")

set(_deps "CisEngine")
add_optional_deps(_deps "jemalloc")

ExternalProject_Add(base
  SOURCE_DIR ${base_source}
  BINARY_DIR ${base_build}
  CMAKE_CACHE_ARGS
    ${CisEngine_DEFAULT_ARGS}
    ${CisEngine_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)