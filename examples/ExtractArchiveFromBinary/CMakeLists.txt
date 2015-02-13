# ExtractArchiveFromBinary CMake file

set (HEADER_FILES
)

set (SOURCE_FILES
	ExtractArchiveFromBinary.c
)

add_executable(ExtractArchiveFromBinary ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(ExtractArchiveFromBinary skyutils)
