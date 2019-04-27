
cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME test_task)
file(GLOB SRCLIST ./src/*.*)

add_executable(${PROJ_NAME} ${SRCLIST})

include_directories(
	./src/
	${BASE_DIR}/src
	${MODULE_DIR}/modules/src)


set(dl dl)
set(pthread pthread)


target_link_libraries(${PROJ_NAME} 
	base
	${dl}
	${pthread}
	)

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "advanced.demo") 