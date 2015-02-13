# debug_sample CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	debug_sample.c
)

add_executable(debug_sample ${HEADER_FILES} ${SOURCE_FILES})

set(LINK_LIBRARIES skyutils)
if(WIN32)
	set(LINK_LIBRARIES ${LINK_LIBRARIES} ws2_32)
else()
	set(LINK_LIBRARIES ${LINK_LIBRARIES} dl m)
endif()

target_link_libraries(debug_sample ${LINK_LIBRARIES})
