set(component_source "${CMAKE_CURRENT_SOURCE_DIR}/component")
set(component_build "${CMAKE_CURRENT_BINARY_DIR}/component")

set(_deps "base")
add_optional_deps(_deps "boost")

ExternalProject_Add(components
  SOURCE_DIR ${component_source}
  BINARY_DIR ${component_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add wolf components properties so correct version.
      "-DComponent_INCLUDE_DIR:PATH=${base_source}/src"
      "-DComponent_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}/component")

#add_custom_command(TARGET base POST_BUILD
#                 COMMAND ${CMAKE_COMMAND} -E copy
#                   ${component_build}/*.so ${LIBRARY_OUTPUT_PATH}/component)

if(FORCE_STEP)
  ExternalProject_Add_Step(components forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of wolf components"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()