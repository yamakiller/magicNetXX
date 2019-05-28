set(cppformat_source "${CMAKE_CURRENT_SOURCE_DIR}/cppformat-master")
set(cppformat_build "${CMAKE_CURRENT_BINARY_DIR}/cppformat-master")


ExternalProject_Add(cppformat
  SOURCE_DIR ${cppformat_source}
  BINARY_DIR ${cppformat_build}
  INSTALL_COMMAND ""
  CMAKE_ARGS -DBUILD_SHARED_LIBS=ON
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add cppformat engine properties so correct version .
      "-DCppformat_INCLUDE_DIR:PATH=${cppformat_source}"
      "-DCppformat_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")

add_custom_command(TARGET cppformat POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${cppformat_build}/libcppformat.* ${LIBRARY_OUTPUT_PATH})


if(FORCE_STEP)
  ExternalProject_Add_Step(cppformat forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of cppformat 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
