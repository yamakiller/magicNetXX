set(base_source "${CMAKE_CURRENT_SOURCE_DIR}/base")
set(base_build "${CMAKE_CURRENT_BINARY_DIR}/base")

add_optional_deps(_deps "boost" "jemalloc" "cppformat" "spdlog" "tinyxml2")


ExternalProject_Add(base
  SOURCE_DIR ${base_source}
  BINARY_DIR ${base_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${CisEngine_DEFAULT_ARGS}
    ${CisEngine_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)

list(APPEND CisEngine_THIRDPARTYLIBS_ARGS
    # Add base engine properties so correct version of Boost is found.
      "-DBase_INCLUDE_DIR:PATH=${base_source}/src"
      "-DBase_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")


add_custom_command(TARGET base POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                   ${base_build}/*.so ${LIBRARY_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(base forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of base sdk"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()

