# su_dbg_console CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	su_dbg_console.c
)

add_executable(su_dbg_console ${HEADER_FILES} ${SOURCE_FILES})

set(LINK_LIBRARIES skyutils)
if(WIN32)
	set(LINK_LIBRARIES ${LINK_LIBRARIES} ws2_32)
endif()

target_link_libraries(su_dbg_console ${LINK_LIBRARIES})
