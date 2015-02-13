# Samples CMake file

add_subdirectory(AppendArchiveToBinary)
add_subdirectory(CreateArchive)
if(WIN32)
	add_subdirectory(ExtractArchive)
endif()
add_subdirectory(ExtractArchiveFromBinary)
add_subdirectory(TestArchive)
add_subdirectory(TestBuffer)
add_subdirectory(TestThread)
add_subdirectory(debug_sample)
add_subdirectory(registry_sample)
