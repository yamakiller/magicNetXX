
cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME base)
file(GLOB 
	SRCLIST 
	./src/*.* 
	./src/util/*.* 
	./src/log/*.* 
	./src/operation/*.* 
	./src/module/*.* 
	./src/network/*.*
	./src/test/*.*)

add_library(${PROJ_NAME} SHARED ${SRCLIST})
add_definitions(-DUSE_JEMALLOC)

find_package(Boost COMPONENTS system coroutine context REQUIRED)
find_package(spdlog)
find_library(jemalloc ${Jemalloc_LIBRARIES})
set(jemalloc ${Jemalloc_LIBRARIES}/libjemalloc.so)
set(cppformat ${Cppformat_LIBRARIES}/libcppformat.so)



include_directories(
	./src
	${Spdlog_INCLUDE_DIR}
	${Cppformat_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${Jemalloc_INCLUDE_DIR}
)



target_link_libraries(${PROJ_NAME} 
	dl
	pthread
	spdlog::spdlog
	${cppformat}
	${jemalloc}
	${Boost_LIBRARIES})

set_target_properties(${PROJ_NAME} PROPERTIES CXX_STANDARD 17 FOLDER "core")

