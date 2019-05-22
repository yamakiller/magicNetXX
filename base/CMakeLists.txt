
cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME base)
file(GLOB SRCLIST ./src/*.*)

add_library(${PROJ_NAME} SHARED ${SRCLIST})
add_definitions(-DUSE_JEMALLOC)

find_package(Boost COMPONENTS system coroutine context REQUIRED)
find_library(jemalloc ${Jemalloc_LIBRARIES})
set(jemalloc ${Jemalloc_LIBRARIES}/libjemalloc.so)


include_directories(
	${Cppformat_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${Jemalloc_INCLUDE_DIR}
)


target_link_libraries(${PROJ_NAME} 
	dl
	pthread
	${jemalloc}
	${Boost_LIBRARIES})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "core")

