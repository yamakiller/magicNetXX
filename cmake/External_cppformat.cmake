set(cppformat_source "${CMAKE_CURRENT_SOURCE_DIR}/cppformat-master")
set(cppformat_build "${CMAKE_CURRENT_BINARY_DIR}/cppformat-master")


ExternalProject_Add(cppformat
  SOURCE_DIR ${cppformat_source}
  BINARY_DIR ${cppformat_build}
  INSTALL_COMMAND ""
  CMAKE_ARGS -DBUILD_SHARED_LIBS=ON
  CMAKE_CACHE_ARGS
    ${CisEngine_DEFAULT_ARGS}
    ${CisEngine_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)

list(APPEND CisEngine_THIRDPARTYLIBS_ARGS
    # Add launcher engine properties so correct version of Boost is found.
      "-DCppformat_INCLUDE_DIR:PATH=${cppformat_source}"
      "-DCppformat_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")

add_custom_command(TARGET cppformat POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${cppformat_build}/*.so ${LIBRARY_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(cppformat forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of cppformat 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
