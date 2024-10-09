cmake_minimum_required(VERSION 3.26)

set(sources
	"${CMAKE_CURRENT_SOURCE_DIR}/Window.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(
	core
	${sources}
)

if(MSVC)
	target_compile_options(
		core
		PRIVATE /W4
		PRIVATE /MP
		PRIVATE /WX
	)
endif()

target_include_directories(
	core
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)