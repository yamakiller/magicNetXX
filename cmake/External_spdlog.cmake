#set(spdlog_source "${CMAKE_CURRENT_SOURCE_DIR}/spdlog")
#set(spdlog_build "${CMAKE_CURRENT_BINARY_DIR}/spdlog")

ExternalProject_Add(spdlog
  DOWNLOAD_DIR ${download_dir}
  URL https://github.com/gabime/spdlog/archive/v1.3.1.tar.gz
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/spdlog"
  INSTALL_DIR "${CisEngine_INSTALL_PREFIX}"
  CMAKE_ARGS ${Wolf_DEFAULT_ARGS} ${Wolf_THIRDPARTYLIBS_ARGS} -DSPDLOG_BUILD_BENCH=OFF -DSPDLOG_BUILD_TESTS=OFF -DSPDLOG_BUILD_EXAMPLES=OFF
)


list(APPEND Wolf_THIRDPARTYLIBS_ARGS
# Add Spdlog properties so correct version of Boost is found.
  "-DSpdlog_INCLUDE_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/spdlog/include"
  "-DSpdlog_LIBRARIES:PATH=${CMAKE_CURRENT_BINARY_DIR}/spdlog")

if(FORCE_STEP)
  ExternalProject_Add_Step(spdlog forcebuild
    COMMAND ${CMAKE_COMMAND} -E echo "Force build of spdlog 3rd"
    ${FORCE_STEP_ARGS}
    ALWAYS 1)
endif()
