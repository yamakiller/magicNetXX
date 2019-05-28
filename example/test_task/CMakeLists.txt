
cmake_minimum_required(VERSION 2.8)

set(PROJ_NAME test_task)
file(GLOB SRCLIST ./src/*.*)

add_executable(${PROJ_NAME} ${SRCLIST})

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system coroutine context REQUIRED)

include_directories(
	./src/
	${Base_INCLUDE_DIR}
	${Launcher_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS})


set(dl dl)
set(pthread pthread)
set(base ${Base_LIBRARIES}/libbase.so)
set(launcher ${Launcher_LIBRARIES}/liblauncher.a)
set(jemalloc ${Jemalloc_LIBRARIES}/libjemalloc.so)
set(cppformat ${Cppformat_LIBRARIES}/libcppformat.so)


target_link_libraries(${PROJ_NAME} 
	${base}
	${cppformat}
	${launcher}
	${jemalloc}
	${dl}
	${pthread})

set_target_properties(${PROJ_NAME} PROPERTIES FOLDER "advanced.demo") 