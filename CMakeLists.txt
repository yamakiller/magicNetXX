
cmake_minimum_required(VERSION 3.10)

project(CisEngine)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  ${CMAKE_MODULE_PATH})


set(CMAKE_BUILD_TYPE Debug)

include(BuildType)
include(BuildLocation)
include(ExternalProject)

if(UNIX)
set(CMAKE_C_FLAGS_DEBUG		"-O0 -Wall -g2 -ggdb")
set(CMAKE_C_FLAGS_RELEASE	"-Wall -o4")
set(CMAKE_CXX_FLAGS_DEBUG	"-std=c++17 -O0 -Wall -g2 -ggdb -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE	"-std=c++17 -Wall -o4 -fPIC")
ELSEIF(WIN32)
add_definitions(-W2)
endif(UNIX)

IF(WIN32)
	set(ws2_32 ws2_32)
ELSEIF(UNIX)
	set(dl dl)
    set(jemalloc jemalloc)
	set(pthread pthread)
ENDIF(WIN32)

set(ROOT_DIR ${PROJECT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(THIRDLIB_DIR ${PROJECT_SOURCE_DIR}/3rd)

set(CisEngine_INSTALL_PREFIX "${CisEngine_BINARY_DIR}/prefix")
set(CisEngine_DEFAULT_ARGS
  "-DCMAKE_PREFIX_PATH:PATH=${CisEngine_INSTALL_PREFIX};${CMAKE_PREFIX_PATH}"
  "-DCMAKE_INSTALL_PREFIX:PATH=${CisEngine_INSTALL_PREFIX}"
  "-DCMAKE_CXX_COMPILER:PATH=${CMAKE_CXX_COMPILER}"
 )

set(download_dir "${CMAKE_CURRENT_BINARY_DIR}/Downloads")


set(FORCE_STEP_DEFAULT "build")
if(CMAKE_CONFIGURATION_TYPES)
  set(FORCE_STEP_DEFAULT "configure")
endif()

set(FORCE_STEP ${FORCE_STEP_DEFAULT} CACHE STRING
  "Should the top level projects force configure/build/install each make")
set_property(CACHE FORCE_STEP PROPERTY STRINGS OFF configure build install)

if(FORCE_STEP STREQUAL configure)
  set(dependee "update")
elseif(FORCE_STEP STREQUAL build)
  set(dependee "configure")
elseif(FORCE_STEP STREQUAL install)
  set(dependee "build")
endif()

set(FORCE_STEP_ARGS
  DEPENDEES ${dependee}
  DEPENDERS ${FORCE_STEP}
)

macro(add_optional_deps dep_var)
  foreach(_dependency ${ARGN})
    string(TOUPPER "${_dependency}" _uDependency)
      list(APPEND ${dep_var} ${_dependency})
  endforeach()
endmacro()

set(base
	  launcher
	  cppformat

	  ws2_32
)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

#add_subdirectory(launcher)
add_subdirectory(3rd)

include(${ROOT_DIR}/cmake/External_base.cmake)
include(${ROOT_DIR}/cmake/External_launcher.cmake)
include(${ROOT_DIR}/cmake/External_example.cmake)
#add_subdirectory(base)
#add_subdirectory(module)
#add_subdirectory(example)



