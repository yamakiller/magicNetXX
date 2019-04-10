
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME base)
file(GLOB SRCLIST ./src/*.*)

add_library(${PROJ_NAME} ${SRCLIST})
add_definitions(-DUSE_JEMALLOC)

include_directories(
	${THIRDLIB_DIR}/cppformat-master
)

target_link_libraries(${PROJ_NAME} 
	libjemalloc)

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "core")

