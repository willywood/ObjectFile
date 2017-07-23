# Microsoft Developer Studio Project File - Name="ofilelib_ole" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ofilelib_ole - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ofilelib_ole.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ofilelib_ole.mak" CFG="ofilelib_ole - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ofilelib_ole - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ofilelib_ole - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ofilelib_ole - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40d /d "NDEBUG"
# ADD RSC /l 0x40d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ofilelib_ole - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GR /GX /Zi /O2 /X /I "../../../stl" /I "D:\Program Files\Microsoft Visual Studio\VC98\Include" /I "../../../ofile" /I "../../../test" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "OFILE_STD_IN_NAMESPACE" /D "OF_OLE" /YX /FD /c
# ADD BASE RSC /l 0x40d /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ofilelib_ole - Win32 Release"
# Name "ofilelib_ole - Win32 Debug"
# Begin Source File

SOURCE=..\..\..\ofile\ConvertUTF.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oblob.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oblobp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\obuf.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oconvert.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\ofile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\ofile2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oflist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oistrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oisxml.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oiter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\ometa.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oosxml.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\opersist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\ostrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\oufile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\ofile\ox.cpp
# End Source File
# End Target
# End Project
