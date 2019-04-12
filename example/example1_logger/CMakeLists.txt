cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME example1_logger)

file(GLOB SRCLIST ./src/*.*)

add_executable(${PROJ_NAME} ${SRCLIST})

IF(UNIX)
	set(dl dl)
	set(pthread pthread)
ENDIF(UNIX)

include_directories(
	./src/
	${BASE_DIR}/src
	${LAUNCHER_DIR}/src
    ${THIRDLIB_DIR}/cppformat-master)
    
target_link_libraries(${PROJ_NAME} 
	base
	launcher
	cppformat
	${dl}
	${pthread})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "logger.example") 
