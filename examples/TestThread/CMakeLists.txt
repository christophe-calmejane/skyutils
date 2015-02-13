# TestThread CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	TestThread.c
)

add_executable(TestThread ${HEADER_FILES} ${SOURCE_FILES})

set(LINK_LIBRARIES skyutils)
if(WIN32)
	set(LINK_LIBRARIES ${LINK_LIBRARIES} ws2_32)
endif()

target_link_libraries(TestThread ${LINK_LIBRARIES})
