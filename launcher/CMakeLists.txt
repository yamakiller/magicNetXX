
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME launcher)
file(GLOB SRCLIST ./src/*.*)

find_package(Boost COMPONENTS system coroutine context REQUIRED)

add_library(${PROJ_NAME} ${SRCLIST})



include_directories(
	./src
	${Cppformat_INCLUDE_DIR}
	${Base_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

target_link_libraries(${PROJ_NAME} base)

set_target_properties(${PROJ_NAME} PROPERTIES CXX_STANDARD 17 FOLDER "core") 
