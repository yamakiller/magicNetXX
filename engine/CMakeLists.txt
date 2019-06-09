
cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME engine)
file(GLOB 
	SRCLIST 
	./src/*.* 
	./src/util/*.* 
	./src/log/*.* 
	./src/operation/*.* 
	./src/module/*.* 
	./src/network/*.*
	./src/api/*.*
	./src/lualib/auto/*.*
	./src/lualib/manual/*.*)

add_library(${PROJ_NAME} SHARED ${SRCLIST})
add_definitions(-DUSE_JEMALLOC)

find_package(Boost COMPONENTS system coroutine context REQUIRED)
find_package(spdlog)
find_library(jemalloc ${Jemalloc_LIBRARIES})
set(lua ${LUA_LIBRARIES})
set(tolua ${Tolua_LIBRARIES}/libtolua++.a)
set(jemalloc ${Jemalloc_LIBRARIES}/libjemalloc.so)
set(cppformat ${Cppformat_LIBRARIES}/libcppformat.so)
set(tinyxml2 ${Tinyxml2_LIBRARIES}/libtinyxml2.so)



include_directories(
	./src
	${LUA_INCLUDE_DIR}
	${Tolua_INCLUDE_DIR}
	${Tinyxml2_INCLUDE_DIR}
	${Spdlog_INCLUDE_DIR}
	${Cppformat_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${Jemalloc_INCLUDE_DIR}
)



target_link_libraries(${PROJ_NAME} 
	dl
	pthread
	spdlog::spdlog
	${lua}
	${tolua}
	${tinyxml2}
	${cppformat}
	${jemalloc}
	${Boost_LIBRARIES})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "core")

