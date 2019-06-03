if("${CMAKE_CURRENT_BINARY_DIR}" MATCHES " ")
  message(FATAL_ERROR "cannot use python configure with a space in the name of the build dir")
endif()

set(python_source "${CMAKE_CURRENT_SOURCE_DIR}/python")
set(python_build "${CMAKE_CURRENT_BINARY_DIR}/python")
#--enable-optimizations
ExternalProject_Add(python
    SOURCE_DIR ${python_source}
    BINARY_DIR ${python_build}
    CONFIGURE_COMMAND COMMAND ${CMAKE_COMMAND} -E copy_directory ${python_source} ${python_build}
    BUILD_COMMAND ${MAKE}
    INSTALL_DIR "${CisEngine_INSTALL_PREFIX}"
    INSTALL_COMMAND ""
    CMAKE_CACHE_ARGS
    ${Wolf_DEFAULT_ARGS}
    ${Wolf_THIRDPARTYLIBS_ARGS}
    )

list(APPEND Wolf_THIRDPARTYLIBS_ARGS
    # Add cppformat pythons properties so correct version .
      "-DPython_INCLUDE_DIR:PATH=${python_source}"
      "-DPython_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")

add_custom_command(TARGET python POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${python_build}/*.so ${python_build}/*.so* ${LIBRARY_OUTPUT_PATH})

if(FORCE_STEP)
  ExternalProject_Add_Step(python forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of python 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()