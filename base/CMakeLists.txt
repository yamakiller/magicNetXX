
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME base)
file(GLOB SRCLIST ./src/*.*)

add_library(${PROJ_NAME} SHARED ${SRCLIST})
add_definitions(-DUSE_JEMALLOC)

find_package(Boost COMPONENTS system coroutine context REQUIRED)

include_directories(
	${THIRDLIB_DIR}/cppformat-master
	${THIRDLIB_DIR}/spdlog/include
	${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJ_NAME} 
	dl
	pthread
	spdlog
	${Boost_LIBRARIES})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "core")

