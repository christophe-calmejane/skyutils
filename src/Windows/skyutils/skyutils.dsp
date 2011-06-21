# Microsoft Developer Studio Project File - Name="skyutils" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=skyutils - Win32 Debug SSL DLL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "skyutils.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "skyutils.mak" CFG="skyutils - Win32 Debug SSL DLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "skyutils - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Debug SSL" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Release SSL" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Release DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Debug DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Release SSL DLL" (based on "Win32 (x86) Static Library")
!MESSAGE "skyutils - Win32 Debug SSL DLL" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "skyutils - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /Zp4 /MT /W3 /O2 /I "..\..\misc\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /Zp4 /MTd /W3 /Zi /Od /I "..\..\misc\include" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Debug SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "skyutils___Win32_Debug_SSL"
# PROP BASE Intermediate_Dir "skyutils___Win32_Debug_SSL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_SSL"
# PROP Intermediate_Dir "Debug_SSL"
# PROP Target_Dir ""
# ADD BASE CPP /MTd /W3 /Zi /Od /I "..\..\..\..\misc\windows\include" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /YX /c
# ADD CPP /MTd /W3 /Zi /Od /I "..\..\misc\include" /D "_DEBUG" /D "DEBUG" /D "SU_USE_SSL" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /YX /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# SUBTRACT BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Release SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "skyutils___Win32_Release_SSL"
# PROP BASE Intermediate_Dir "skyutils___Win32_Release_SSL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_SSL"
# PROP Intermediate_Dir "Release_SSL"
# PROP Target_Dir ""
# ADD BASE CPP /MT /W3 /O2 /I "..\..\..\..\misc\windows\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD CPP /MT /W3 /O2 /I "..\..\misc\include" /D "NDEBUG" /D "SU_USE_SSL" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# SUBTRACT BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Release DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "skyutils___Win32_Release_DLL"
# PROP BASE Intermediate_Dir "skyutils___Win32_Release_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "skyutils___Win32_Release_DLL"
# PROP Intermediate_Dir "skyutils___Win32_Release_DLL"
# PROP Target_Dir ""
# ADD BASE CPP /Zp4 /MT /W3 /O2 /I "..\..\..\..\misc\windows\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD CPP /Zp4 /MD /W3 /O2 /I "..\..\misc\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# SUBTRACT BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Debug DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "skyutils___Win32_Debug_DLL"
# PROP BASE Intermediate_Dir "skyutils___Win32_Debug_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "skyutils___Win32_Debug_DLL"
# PROP Intermediate_Dir "skyutils___Win32_Debug_DLL"
# PROP Target_Dir ""
# ADD BASE CPP /Zp4 /MTd /W3 /Zi /Od /I "..\..\..\..\misc\windows\include" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD CPP /Zp4 /MDd /W3 /Zi /Od /I "..\..\misc\include" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# SUBTRACT BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Release SSL DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "skyutils___Win32_Release_SSL_DLL"
# PROP BASE Intermediate_Dir "skyutils___Win32_Release_SSL_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "skyutils___Win32_Release_SSL_DLL"
# PROP Intermediate_Dir "skyutils___Win32_Release_SSL_DLL"
# PROP Target_Dir ""
# ADD BASE CPP /MT /W3 /O2 /I "..\..\..\..\misc\windows\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /D "SU_USE_SSL" /FR /YX /c
# ADD CPP /MD /W3 /O2 /I "..\..\misc\include" /D "NDEBUG" /D "SU_USE_SSL" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /FR /YX /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# SUBTRACT BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ELSEIF  "$(CFG)" == "skyutils - Win32 Debug SSL DLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "skyutils___Win32_Debug_SSL_DLL"
# PROP BASE Intermediate_Dir "skyutils___Win32_Debug_SSL_DLL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "skyutils___Win32_Debug_SSL_DLL"
# PROP Intermediate_Dir "skyutils___Win32_Debug_SSL_DLL"
# PROP Target_Dir ""
# ADD BASE CPP /MTd /W3 /Zi /Od /I "..\..\..\..\misc\windows\include" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /D "SU_USE_SSL" /YX /c
# ADD CPP /MDd /W3 /Zi /Od /I "..\..\misc\include" /D "_DEBUG" /D "DEBUG" /D "SU_USE_SSL" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /YX /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# SUBTRACT BASE LIB32 /nologo
# SUBTRACT LIB32 /nologo

!ENDIF 

# Begin Target

# Name "skyutils - Win32 Release"
# Name "skyutils - Win32 Debug"
# Name "skyutils - Win32 Debug SSL"
# Name "skyutils - Win32 Release SSL"
# Name "skyutils - Win32 Release DLL"
# Name "skyutils - Win32 Debug DLL"
# Name "skyutils - Win32 Release SSL DLL"
# Name "skyutils - Win32 Debug SSL DLL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\arch.c
DEP_CPP_ARCH_=\
	"..\..\misc\include\bzlib.h"\
	"..\..\misc\include\lzoconf.h"\
	"..\..\misc\include\lzodefs.h"\
	"..\..\misc\include\minilzo.h"\
	"..\..\misc\include\zconf.h"\
	"..\..\misc\include\zlib.h"\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\debug.c
DEP_CPP_DEBUG=\
	"..\..\debug.h"\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\debug_config_w32.c
DEP_CPP_DEBUG_=\
	"..\..\debug.h"\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\liste.c
DEP_CPP_LISTE=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\memory.c
DEP_CPP_MEMOR=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\Misc\src\minilzo.c
DEP_CPP_MINIL=\
	"..\..\misc\include\lzoconf.h"\
	"..\..\misc\include\lzodefs.h"\
	"..\..\misc\include\minilzo.h"\
	
NODEP_CPP_MINIL=\
	"..\..\Misc\src\lzo\lzo1x.h"\
	"..\..\Misc\src\lzo\lzoconf.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\registry.c
DEP_CPP_REGIS=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=.\skyutils.rc
# End Source File
# Begin Source File

SOURCE=..\..\socks.c
DEP_CPP_SOCKS=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\string.c
DEP_CPP_STRIN=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\threads.c
DEP_CPP_THREA=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\utils.c
DEP_CPP_UTILS=\
	"..\..\skyutils.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\web.c
DEP_CPP_WEB_C=\
	"..\..\skyutils.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\skyutils.h
# End Source File
# Begin Source File

SOURCE=.\su_resource.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\misc\windows\lib\libbz2.lib
# End Source File
# Begin Source File

SOURCE="..\..\Misc\lib\zlib-static.lib"
# End Source File
# End Group
# End Target
# End Project
