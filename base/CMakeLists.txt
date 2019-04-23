
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME base)
file(GLOB SRCLIST ./src/*.*)

add_library(${PROJ_NAME} SHARED ${SRCLIST})
add_definitions(-DUSE_JEMALLOC)

include_directories(
	${THIRDLIB_DIR}/cppformat-master
	${THIRDLIB_DIR}/spdlog/include
)

target_link_libraries(${PROJ_NAME} 
	dl
	pthread
	spdlog
	libjemalloc
	boost_system
	boost_coroutine
	boost_context)

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "core")

