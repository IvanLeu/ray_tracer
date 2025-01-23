cmake_minimum_required(VERSION 3.26)

set(sources
	"${CMAKE_CURRENT_SOURCE_DIR}/Graphics.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Graphics.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Application.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Application.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Input.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Input.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Window.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Window.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

file(GLOB SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(
	core
	WIN32
	${sources}
	${SHADER_FILES}
)

if(MSVC)
	target_compile_options(
		core
		PRIVATE /W4
		PRIVATE /MP
	)
endif()

target_link_libraries(
	core
	PRIVATE
	d3d12
	dxgi
	dxguid
	user32
	d3dcompiler
)

target_include_directories(
	core
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/d3dx12
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(SHADERS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_DEST_DIR "$<TARGET_FILE_DIR:core>/shaders")

add_custom_command(TARGET core POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_DEST_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${SHADERS_SRC_DIR}"
		"${SHADERS_DEST_DIR}"
)

source_group("shaders" FILES ${SHADER_FILES})