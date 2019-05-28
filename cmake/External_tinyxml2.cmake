set(tinyxml2_source "${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2")
set(tinyxml2_build "${CMAKE_CURRENT_BINARY_DIR}/tinyxml2")

#CMAKE_ARGS -DBUILD_SHARED_LIBS=ON

ExternalProject_Add(tinyxml2
  SOURCE_DIR ${tinyxml2_source}
  BINARY_DIR ${tinyxml2_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${CisEngine_DEFAULT_ARGS}
    ${CisEngine_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)


list(APPEND CisEngine_THIRDPARTYLIBS_ARGS
    # Add TinyXml2 engine properties so correct version of Boost is found.
      "-DTinyxml2_INCLUDE_DIR:PATH=${tinyxml2_source}"
      "-DTinyxml2_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")

add_custom_command(TARGET tinyxml2 POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                   ${tinyxml2_build}/libtinyxml2.* ${LIBRARY_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(tinyxml2 forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of TinyXml2 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()

