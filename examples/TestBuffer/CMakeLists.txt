# TestBuffer CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	TestBuffer.c
)

add_executable(TestBuffer ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(TestBuffer skyutils)
