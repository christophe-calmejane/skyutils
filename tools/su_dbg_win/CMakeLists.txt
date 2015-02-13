# su_dbg_win CMake file

set (HEADER_FILES
	resource.h
)

set (SOURCE_FILES
	su_dbg_win.c
)

set (RESOURCE_FILES
	su_dbg_win.rc
)

add_executable(su_dbg_win WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

set(LINK_LIBRARIES skyutils)
if(WIN32)
	set(LINK_LIBRARIES ${LINK_LIBRARIES} ws2_32 Shlwapi)
endif()

target_link_libraries(su_dbg_win ${LINK_LIBRARIES})
