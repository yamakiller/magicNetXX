set(engine_source "${CMAKE_CURRENT_SOURCE_DIR}/engine")
set(engine_build "${CMAKE_CURRENT_BINARY_DIR}/engine")

add_optional_deps(_deps "boost" "jemalloc" "cppformat" "spdlog" "tinyxml2" "lua" "tolua")


ExternalProject_Add(engine
  SOURCE_DIR ${engine_source}
  BINARY_DIR ${engine_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)


list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add engine engine properties so correct version of Boost is found.
      "-DEngine_INCLUDE_DIR:PATH=${engine_source}/src"
      "-DEngine_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}"
      "-DCOMP_LIBRARY_OUT_PATH:PATH=${EXECUTABLE_OUTPUT_PATH}")


add_custom_command(TARGET engine POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                   ${engine_build}/*.so ${LIBRARY_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(engine forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of engine sdk"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()

