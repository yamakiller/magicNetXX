set(tolua_source "${CMAKE_CURRENT_SOURCE_DIR}/tolua")
set(tolua_build "${CMAKE_CURRENT_BINARY_DIR}/tolua")

add_optional_deps(_deps "lua")

ExternalProject_Add(tolua
  SOURCE_DIR ${tolua_source}
  BINARY_DIR ${tolua_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add cppformat engine properties so correct version .
      "-DTolua_INCLUDE_DIR:PATH=${tolua_source}/include"
      "-DTolua_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}"
      "-DTolua_RUN:PATH=${EXECUTABLE_OUTPUT_PATH}")

add_custom_command(TARGET tolua POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${tolua_build}/lib/*.a ${LIBRARY_OUTPUT_PATH})

add_custom_command(TARGET tolua POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${tolua_build}/bin/* ${EXECUTABLE_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(tolua forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of tolua 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
