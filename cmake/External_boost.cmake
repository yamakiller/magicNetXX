# Build boost via its bootstrap script. The build tree cannot contain a space.
# This boost b2 build system yields errors with spaces in the name of the
# build dir.
#
if("${CMAKE_CURRENT_BINARY_DIR}" MATCHES " ")
  message(FATAL_ERROR "cannot use boost bootstrap with a space in the name of the build dir")
endif()


#Always use the static libs on Windows.

set(link shared)
set(Boost_USE_STATIC_LIBS OFF)


set(boost_with_args
  --with-system
  --with-coroutine
  --with-context
  --link=${link}
)

set(am 64)

list(APPEND boost_with_args
  "cxxflags=-fPIC")
set(boost_cmds
  CONFIGURE_COMMAND ./bootstrap.sh --prefix=<INSTALL_DIR>
  BUILD_COMMAND ./b2 address-model=${am} ${boost_with_args})

ExternalProject_Add(boost
  DOWNLOAD_DIR ${download_dir}
  URL https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost"
  INSTALL_DIR "${CisEngine_INSTALL_PREFIX}"
  ${boost_cmds}
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1
  CMAKE_CACHE_ARGS
  ${CisEngine_DEFAULT_ARGS}
  ${CisEngine_THIRDPARTYLIBS_ARGS}
)



ExternalProject_Get_Property(boost install_dir)
set(BOOST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/boost" CACHE INTERNAL "")
set(BOOST_LIB "${BOOST_ROOT}/stage/lib" CACHE INTERNAL "")

list(APPEND CisEngine_THIRDPARTYLIBS_ARGS
# Add Boost properties so correct version of Boost is found.
  "-DBOOST_ROOT:PATH=${BOOST_ROOT}"
  "-DBoost_INCLUDE_DIR:PATH=${BOOST_ROOT}"
  "-DBoost_LIBRARIES:PATH=${BOOST_LIB}")

add_custom_command(TARGET boost POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${BOOST_LIB} ${CisEngine_INSTALL_PREFIX}/lib)
