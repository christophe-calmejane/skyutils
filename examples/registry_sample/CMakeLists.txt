# registry_sample CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	registry_sample.c
)

add_executable(registry_sample ${HEADER_FILES} ${SOURCE_FILES})

set(LINK_LIBRARIES skyutils)
if(WIN32)
	set(LINK_LIBRARIES ${LINK_LIBRARIES} ws2_32)
endif()

target_link_libraries(registry_sample ${LINK_LIBRARIES})
