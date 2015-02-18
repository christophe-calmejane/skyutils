# Tools CMake file

include_directories(../include)

if(WIN32)
	add_subdirectory(su_dbg_win)
endif()
add_subdirectory(su_dbg_console)
