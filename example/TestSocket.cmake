set(exampleapp_TestSocket_source "${CMAKE_CURRENT_SOURCE_DIR}/example/TestSocket")
set(exampleapp_TestSocket_build "${CMAKE_CURRENT_BINARY_DIR}/example/bin")

set(_deps "engine")
add_optional_deps(_deps "boost" "launcher")

ExternalProject_Add(exampleapp_TestSocket
   SOURCE_DIR ${exampleapp_TestSocket_source}
   BINARY_DIR ${exampleapp_TestSocket_build}
   INSTALL_COMMAND ""
   CMAKE_CACHE_ARGS
   ${Wolf_DEFAULT_ARGS}
   ${Wolf_THIRDPARTYLIBS_ARGS}
   -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
   ${_deps}
)

if(FORCE_STEP)
  ExternalProject_Add_Step(exampleapp_TestSocket forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of exampleapp_TestSocket"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()