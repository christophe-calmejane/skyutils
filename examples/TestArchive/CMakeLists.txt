# TestArchive CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	TestArchive.c
)

add_executable(TestArchive ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(TestArchive skyutils)
