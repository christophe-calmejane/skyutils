# Library CMake file

#include_directories(${CMAKE_SOURCE_DIR}/include)

set (HEADER_FILES
	${CMAKE_SOURCE_DIR}/include/skyutils/skyutils.h
	${CMAKE_SOURCE_DIR}/include/skyutils/su_resource.h
)

set (SOURCE_FILES_COMMON
	arch.c
	buffer.c
	debug.h
	debug.c
	liste.c
	memory.c
	registry.c
	socks.c
	string.c
	threads.c
	utils.c
	web.c
)

set (SOURCE_FILES_WINDOWS
	debug_config_w32.c
)

set (SOURCE_FILES_LINUX
)

if(WIN32)
	set (SOURCE_FILES ${SOURCE_FILES_COMMON} ${SOURCE_FILES_WINDOWS})
else()
	set (SOURCE_FILES ${SOURCE_FILES_COMMON} ${SOURCE_FILES_LINUX})
endif()

add_library(skyutils ${HEADER_FILES} ${SOURCE_FILES})

if(CMAKE_COMPILER_IS_GNUCC)
	set_property(TARGET skyutils APPEND_STRING PROPERTY COMPILE_FLAGS -Wall)
endif()
if(MSVC)
	set_property(TARGET skyutils APPEND_STRING PROPERTY COMPILE_FLAGS /WX)
endif()

