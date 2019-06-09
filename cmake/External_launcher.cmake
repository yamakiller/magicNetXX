set(launcher_source "${CMAKE_CURRENT_SOURCE_DIR}/launcher")
set(launcher_build "${CMAKE_CURRENT_BINARY_DIR}/launcher")

set(_deps "engine")

ExternalProject_Add(launcher
  SOURCE_DIR ${launcher_source}
  BINARY_DIR ${launcher_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add launcher engine properties so correct version of Boost is found.
      "-DLauncher_INCLUDE_DIR:PATH=${launcher_source}/src"
      "-DLauncher_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")

add_custom_command(TARGET launcher POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${launcher_build}/*.a ${LIBRARY_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(launcher forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of launcher sdk"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
