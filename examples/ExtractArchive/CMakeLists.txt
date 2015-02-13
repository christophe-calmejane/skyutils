# ExtractArchive CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	ExtractArchive.c
)

add_executable(ExtractArchive ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(ExtractArchive skyutils)
