set(lualib_source "${CMAKE_CURRENT_SOURCE_DIR}/lualib")
set(lualib_build  "${CMAKE_CURRENT_BINARY_DIR}/lualib")

add_optional_deps(_deps "tolua")

ExternalProject_Add(lualib
  SOURCE_DIR ${lualib_source}
  BINARY_DIR ${lualib_build}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
    ${_deps}
)


if(FORCE_STEP)
  ExternalProject_Add_Step(lualib forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of lualib"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()

