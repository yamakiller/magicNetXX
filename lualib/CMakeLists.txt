cmake_minimum_required(VERSION 3.14)

set(PROJ_NAME lualib)
file(GLOB SRCLIST ./src/auto/*.* ./src/manual/*.*)

find_package(Boost COMPONENTS system coroutine context REQUIRED)

add_library(${PROJ_NAME} ${SRCLIST})

include_directories(
    ./src
    ${Tolua_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
	${Cppformat_INCLUDE_DIR}
	${Base_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

set(base ${Base_LIBRARIES}/libbase.so)
#set(lua  ${LUA_LIBRARIES}/liblua.a)
set(tolua ${Tolua_LIBRARIES}/libtolua.a)

target_link_libraries(${PROJ_NAME} 
                      ${base}
                      ${tolua})

set_target_properties(${PROJ_NAME} PROPERTIES CXX_STANDARD 11 FOLDER "lualib") 
