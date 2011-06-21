# Microsoft Developer Studio Project File - Name="skyutilsdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=skyutilsdll - Win32 Debug SSL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "skyutilsdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "skyutilsdll.mak" CFG="skyutilsdll - Win32 Debug SSL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "skyutilsdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "skyutilsdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "skyutilsdll - Win32 Debug SSL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "skyutilsdll - Win32 Release SSL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "skyutilsdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\misc\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /D "SU_USE_DL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "skyutilsdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\misc\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /D "SU_USE_DL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "skyutilsdll - Win32 Debug SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug SSL"
# PROP BASE Intermediate_Dir "Debug SSL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug SSL"
# PROP Intermediate_Dir "Debug SSL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\misc\include" /D "_DEBUG" /D "SU_USE_SSL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /D "SU_USE_DL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "skyutilsdll - Win32 Release SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release SSL"
# PROP BASE Intermediate_Dir "Release SSL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release SSL"
# PROP Intermediate_Dir "Release SSL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "SU_ENABLE_ANSI_CODE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\misc\include" /D "NDEBUG" /D "SU_USE_SSL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYUTILSDLL_EXPORTS" /D "_REENTRANT" /D "SU_USE_ARCH" /D "HAVE_ZLIB" /D "HAVE_BZLIB" /D "HAVE_MINILZO" /D "SU_ENABLE_ANSI_CODE" /D "SU_USE_DL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ENDIF 

# Begin Target

# Name "skyutilsdll - Win32 Release"
# Name "skyutilsdll - Win32 Debug"
# Name "skyutilsdll - Win32 Debug SSL"
# Name "skyutilsdll - Win32 Release SSL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\arch.c
# End Source File
# Begin Source File

SOURCE=..\..\debug.c
# End Source File
# Begin Source File

SOURCE=..\..\debug_config_w32.c
# End Source File
# Begin Source File

SOURCE=..\..\liste.c
# End Source File
# Begin Source File

SOURCE=..\..\memory.c
# End Source File
# Begin Source File

SOURCE=..\..\Misc\src\minilzo.c
# End Source File
# Begin Source File

SOURCE=..\..\registry.c
# End Source File
# Begin Source File

SOURCE=..\skyutils\skyutils.rc
# End Source File
# Begin Source File

SOURCE=..\..\socks.c
# End Source File
# Begin Source File

SOURCE=..\..\string.c
# End Source File
# Begin Source File

SOURCE=..\..\threads.c
# End Source File
# Begin Source File

SOURCE=..\..\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\web.c
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

SOURCE=..\skyutils\su_resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE="..\..\Misc\lib\zlib-static.lib"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\misc\windows\lib\libbz2.lib
# End Source File
# End Group
# End Target
# End Project
