cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME module-logger)
file(GLOB SRCLIST ./src/*.*)


add_library(${PROJ_NAME} SHARED ${SRCLIST})

include_directories(
	${BASE_DIR}/src
	${THIRDLIB_DIR}/cppformat-master
	${THIRDLIB_DIR}/spdlog-master/include/spdlog
)

target_link_libraries(${PROJ_NAME} 
                      base)

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "modules") 
set_target_properties(${PROJ_NAME} PROPERTIES PREFIX "")

