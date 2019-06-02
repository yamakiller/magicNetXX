cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME wolf_python)
file(GLOB 
	SRCLIST 
    ./src/*.*)
    
add_library(${PROJ_NAME} SHARED ${SRCLIST})
#add_definitions(-DUSE_JEMALLOC)

find_package(Boost COMPONENTS system coroutine context REQUIRED)
include_directories(
	./src/
	${Base_INCLUDE_DIR}
	${Launcher_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS})

set(base ${Base_LIBRARIES}/libbase.so)

target_link_libraries(${PROJ_NAME} 
    ${base})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER 
    "scriptLibs" 
    PREFIX "")
