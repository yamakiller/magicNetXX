
cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME SocketTest)
file(GLOB SRCLIST ./src/*.*)

add_library(${PROJ_NAME}  SHARED ${SRCLIST})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system coroutine context REQUIRED)

include_directories(
	./src/
	${Engine_INCLUDE_DIR}
	${LUA_INCLUDE_DIR}
	${Cppformat_INCLUDE_DIR}
	${Launcher_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS})
    
set(engine ${Engine_LIBRARIES}/libengine.so)

target_link_libraries(${PROJ_NAME} 
	${engine})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER 
									"components" 
									PREFIX "")


add_custom_command(TARGET ${PROJ_NAME} POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_BINARY_DIR}/*.so ${COMP_LIBRARY_OUT_PATH}/components/)