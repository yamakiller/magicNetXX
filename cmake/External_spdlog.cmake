set(spdlog_source "${CMAKE_CURRENT_SOURCE_DIR}/spdlog")
set(spdlog_build "${CMAKE_CURRENT_BINARY_DIR}/spdlog")

# CMAKE_CACHE_ARGS ${CMAKE_CACHE_ARGS} "-DSPDLOG_BUILD_TESTS:BOOL=OFF" "-DSPDLOG_BUILD_EXAMPLES:BOOL=OFF" "-DSPDLOG_BUILD_BENCH:BOOL=OFF"
ExternalProject_Add(spdlog
  SOURCE_DIR ${spdlog_source}
  BINARY_DIR ${spdlog_build}
  CMAKE_CACHE_ARGS ${CMAKE_CACHE_ARGS} "-DSPDLOG_BUILD_TESTS:BOOL=OFF" "-DSPDLOG_BUILD_EXAMPLES:BOOL=OFF" "-DSPDLOG_BUILD_BENCH:BOOL=OFF"
  CONFIGURE_COMMAND "" 
  BUILD_COMMAND "" 
  INSTALL_COMMAND "" 
  UPDATE_COMMAND "" 
)



list(APPEND CisEngine_THIRDPARTYLIBS_ARGS
    # Add spdlog engine properties so correct version of Boost is found.
      "-DSpdlog_INCLUDE_DIR:PATH=${spdlog_source}/include"
      "-DSpdlog_LIBRARIES:PATH=${LIBRARY_OUTPUT_PATH}")
    
if(FORCE_STEP)
  ExternalProject_Add_Step(spdlog forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of spdlog sdk"
    ${FORCE_STEP_ARGS}
   ALWAYS 1)
endif() 