set(exampleapp_test_task_source "${CMAKE_CURRENT_SOURCE_DIR}/example/test_task")
set(exampleapp_test_task_build "${CMAKE_CURRENT_BINARY_DIR}/example/bin")

set(_deps "base")
add_optional_deps(_deps "boost")

ExternalProject_Add(exampleapp_test_task
   SOURCE_DIR ${exampleapp_test_task_source}
   BINARY_DIR ${exampleapp_test_task_build}
   INSTALL_COMMAND ""
   CMAKE_CACHE_ARGS
   ${CisEngine_DEFAULT_ARGS}
   ${CisEngine_THIRDPARTYLIBS_ARGS}
   -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
  DEPENDS
   ${_deps}
)

if(FORCE_STEP)
  ExternalProject_Add_Step(exampleapp_test_task forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of exampleapp_test_task"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()