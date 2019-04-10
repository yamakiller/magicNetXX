
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME launcher)
file(GLOB SRCLIST ./src/*.*)

add_library(${PROJ_NAME} ${SRCLIST})

include_directories(
	./src
	${BASE_DIR}/src
	${THIRDLIB_DIR}/cppformat-master
	${THIRDLIB_DIR}/spdlog-master/include/spdlog
)

target_link_libraries(${PROJ_NAME} base)

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "core") 
