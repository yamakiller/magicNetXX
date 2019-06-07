set(lua_source "${CMAKE_CURRENT_SOURCE_DIR}/lua")


ExternalProject_Add(lua
  SOURCE_DIR ${lua_source}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make linux
  INSTALL_DIR "${CisEngine_INSTALL_PREFIX}"
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
)

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add cppformat engine properties so correct version .
      "-DLUA_INCLUDE_DIR:PATH=${lua_source}/src"
      "-DLUA_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}/liblua.a")

add_custom_command(TARGET lua POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${lua_source}/src/*.a ${LIBRARY_OUTPUT_PATH})


if(FORCE_STEP)
  ExternalProject_Add_Step(lua forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of lua 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
