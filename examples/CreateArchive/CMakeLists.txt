# CreateArchive CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	CreateArchive.c
)

add_executable(CreateArchive ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(CreateArchive skyutils)
