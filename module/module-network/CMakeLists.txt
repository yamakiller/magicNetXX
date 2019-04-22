
cmake_minimum_required(VERSION 3.10)

set(PROJ_NAME module_network)
file(GLOB SRCLIST ./src/*.*)


add_library(${PROJ_NAME} SHARED ${SRCLIST})

include_directories(
	${BASE_DIR}/src
	${THIRDLIB_DIR}/cppformat-master
)

target_link_libraries(${PROJ_NAME} 
					  base)

if(UNIX)
target_link_libraries(${PROJ_NAME} pthread)
target_link_libraries(${PROJ_NAME} m)
target_link_libraries(${PROJ_NAME} dl)
ELSEIF(WIN32)
target_link_libraries(${PROJ_NAME} ws2_32)
target_link_libraries(${PROJ_NAME} Mswsock)

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "modules") 
set_target_properties(${PROJ_NAME} PROPERTIES PREFIX "")
endif(UNIX)
