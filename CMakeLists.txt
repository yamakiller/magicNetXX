
cmake_minimum_required(VERSION 3.10)

project(cis-server)

set(CMAKE_BUILD_TYPE Debug)

if(UNIX)
set(CMAKE_C_FLAGS_DEBUG		"-Wall -g -pg")
set(CMAKE_C_FLAGS_RELEASE	"-Wall -o4")
set(CMAKE_CXX_FLAGS_DEBUG	"-std=c++11 -Wall -g -pg -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE	"-std=c++11 -Wall -o4 -fPIC")
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
set(BASE_DIR ${PROJECT_SOURCE_DIR}/base)
set(LAUNCHER_DIR ${PROJECT_SOURCE_DIR}/launcher)
set(MODULE_DIR ${PROJECT_SOURCE_DIR}/module)

set(MODULE_INCLUDE_DIRECTORIES
	${BASE_DIR}/src
	${THIRDLIB_DIR}/cppformat-master
	${THIRDLIB_DIR}/spdlog-master/include/spdlog	
	${LAUNCHER_DIR}/src
)

set(MODULE_LINK_LIBRARIES
	base
	launcher
	cppformat

	ws2_32
)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

add_subdirectory(launcher)
add_subdirectory(3rd)
add_subdirectory(base)
add_subdirectory(module)
add_subdirectory(example)

