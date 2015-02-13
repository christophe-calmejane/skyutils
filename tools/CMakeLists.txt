# Tools CMake file

if(WIN32)
	add_subdirectory(su_dbg_win)
endif()
add_subdirectory(su_dbg_console)
