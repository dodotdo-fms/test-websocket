cmake_minimum_required(VERSION 2.8.11)

project(distro-client)


# Compiler options
add_definitions(-Wno-unused-function)
add_definitions(-Wno-unused-variable)
add_definitions(-Wno-unused-but-set-variable)
add_definitions(-Wno-unused-result)
add_definitions(-Wall)
add_definitions(-I${CMAKE_BINARY_DIR}/include)

# Linker options
set(LINK_OPTIONS -pthread)


# File
file(GLOB C_SRCS
	*.c
)


add_executable(distro-client ${RC_SRCS} ${CXX_SRCS} ${C_SRCS})

set_target_properties(distro-client PROPERTIES COMPILE_FLAGS "--coverage")
set_target_properties(distro-client PROPERTIES LINK_FLAGS "--coverage")
set_target_properties(distro-client PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(distro-client
	${CMAKE_BINARY_DIR}/lib/libwebsockets.a
	pthread
	crypto
	z
)
