# Microsoft Developer Studio Project File - Name="SAFMQ Manager Snapin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SAFMQ Manager Snapin - Win32 Release SSL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SAFMQManagerSnapin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SAFMQManagerSnapin.mak" CFG="SAFMQ Manager Snapin - Win32 Release SSL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SAFMQ Manager Snapin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SAFMQ Manager Snapin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SAFMQ Manager Snapin - Win32 Debug SSL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SAFMQ Manager Snapin - Win32 Release SSL" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "..\shared" /I "..\lib" /I "Debug" /D "_DEBUG" /D ATL_TRACE_LEVEL=0 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(OutDir)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=.\Debug\SAFMQManagerSnapin.dll
InputPath=.\Debug\SAFMQManagerSnapin.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SAFMQ_Manager_Snapin___Win32_Release"
# PROP BASE Intermediate_Dir "SAFMQ_Manager_Snapin___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GR /GX /ZI /I "..\..\safmq.0.5.2" /I "..\..\safmq.0.5.2\lib" /I "..\..\safmq.0.5.2\shared" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /I ".." /I "..\shared" /I "..\lib" /I "Release" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib safmqlib.lib Ws2_32.lib ssleay32.lib libeay32.lib /nologo /subsystem:windows /dll /machine:I386 /pdbtype:sept /libpath:"..\..\safmq.0.5.2\lib\Release"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib /nologo /subsystem:windows /dll /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /debug
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=.\Release\SAFMQManagerSnapin.dll
InputPath=.\Release\SAFMQManagerSnapin.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Debug SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SAFMQ_Manager_Snapin___Win32_Debug_SSL"
# PROP BASE Intermediate_Dir "SAFMQ_Manager_Snapin___Win32_Debug_SSL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_SSL"
# PROP Intermediate_Dir "Debug_SSL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\safmq.0.5.2" /I "..\..\safmq.0.5.2\lib" /I "..\..\safmq.0.5.2\shared" /D "_DEBUG" /D ATL_TRACE_LEVEL=0 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "..\shared" /I "..\lib" /I "Debug_SSL" /D "_DEBUG" /D ATL_TRACE_LEVEL=0 /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SAFMQ_SSL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib safmqlib.lib Ws2_32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:".." /libpath:"..\lib\Debug"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib ssleay32.lib libeay32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Performing registration
OutDir=.\Debug_SSL
TargetPath=.\Debug_SSL\SAFMQManagerSnapin.dll
InputPath=.\Debug_SSL\SAFMQManagerSnapin.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Release SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SAFMQ_Manager_Snapin___Win32_Release_SSL"
# PROP BASE Intermediate_Dir "SAFMQ_Manager_Snapin___Win32_Release_SSL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release_SSL"
# PROP Intermediate_Dir "Release_SSL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GR /GX /ZI /I "..\..\safmq.0.5.2" /I "..\..\safmq.0.5.2\lib" /I "..\..\safmq.0.5.2\shared" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /I ".." /I "..\shared" /I "..\lib" /I "Release_SSL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SAFMQ_SSL" /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib safmqlib.lib Ws2_32.lib /nologo /subsystem:windows /dll /machine:I386 /pdbtype:sept /libpath:".." /libpath:"..\lib\Release"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib ssleay32.lib libeay32.lib /nologo /subsystem:windows /dll /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /debug
# Begin Custom Build - Performing registration
OutDir=.\Release_SSL
TargetPath=.\Release_SSL\SAFMQManagerSnapin.dll
InputPath=.\Release_SSL\SAFMQManagerSnapin.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "SAFMQ Manager Snapin - Win32 Debug"
# Name "SAFMQ Manager Snapin - Win32 Release"
# Name "SAFMQ Manager Snapin - Win32 Debug SSL"
# Name "SAFMQ Manager Snapin - Win32 Release SSL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Actor.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorPerms.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorPermsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AddActorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigData.cpp
# End Source File
# Begin Source File

SOURCE=.\CUserEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\Folder.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupAddDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\LocalSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\NewGroupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NewQueueDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NewServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NewUserDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\PasswordDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SAFMQLocalSettingsEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerRoot.cpp
# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerSnapin.cpp
# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerSnapin.def
# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerSnapin.idl

!IF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Debug"

# ADD MTL /h "Debug/SAFMQManagerSnapin.h"

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Release"

# ADD MTL /h "Release\SAFMQManagerSnapin.h"

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Debug SSL"

# ADD MTL /h "Debug_SSL\SAFMQManagerSnapin.h"

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Release SSL"

# ADD MTL /h "Release_SSL\SAFMQManagerSnapin.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerSnapin.rc

!IF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Debug"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Release"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(OutDir)"

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Debug SSL"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /fo"Debug_SSL\SAFMQManagerSnapin.res" /i "$(OutDir)"

!ELSEIF  "$(CFG)" == "SAFMQ Manager Snapin - Win32 Release SSL"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "$(OutDir)"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SafmqServerNode.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerList.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerLoginDLg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\UserAddDialog.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Actor.h
# End Source File
# Begin Source File

SOURCE=.\ActorPerms.h
# End Source File
# Begin Source File

SOURCE=.\ActorPermsDlg.h
# End Source File
# Begin Source File

SOURCE=.\AddActorDlg.h
# End Source File
# Begin Source File

SOURCE=.\CommonControls.h
# End Source File
# Begin Source File

SOURCE=.\ConfigData.h
# End Source File
# Begin Source File

SOURCE=.\CUserEditor.h
# End Source File
# Begin Source File

SOURCE=.\Folder.h
# End Source File
# Begin Source File

SOURCE=.\GroupAddDialog.h
# End Source File
# Begin Source File

SOURCE=.\GroupEditor.h
# End Source File
# Begin Source File

SOURCE=.\LocalSettings.h
# End Source File
# Begin Source File

SOURCE=.\LocalSettingsCP.h
# End Source File
# Begin Source File

SOURCE=.\NewGroupDlg.h
# End Source File
# Begin Source File

SOURCE=.\NewQueueDlg.h
# End Source File
# Begin Source File

SOURCE=.\NewServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\NewUserDlg.h
# End Source File
# Begin Source File

SOURCE=.\Node.h
# End Source File
# Begin Source File

SOURCE=.\PasswordDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SAFMQLocalSettingsEditor.h
# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerRoot.h
# End Source File
# Begin Source File

SOURCE=.\SafmqServerNode.h
# End Source File
# Begin Source File

SOURCE=.\ServerList.h
# End Source File
# Begin Source File

SOURCE=.\ServerLoginDLg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\UserAddDialog.h
# End Source File
# Begin Source File

SOURCE=.\vvec.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ActorPerms.rgs
# End Source File
# Begin Source File

SOURCE=.\CUserEditor.rgs
# End Source File
# Begin Source File

SOURCE=.\res\group.ico
# End Source File
# Begin Source File

SOURCE=.\res\group_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\group_32.bmp
# End Source File
# Begin Source File

SOURCE=.\GroupEditor.rgs
# End Source File
# Begin Source File

SOURCE=.\res\hh.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\queue_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\safmq.ico
# End Source File
# Begin Source File

SOURCE=.\res\safmq_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\safmq_32.bmp
# End Source File
# Begin Source File

SOURCE=.\SAFMQLocalSettingsEditor.rgs
# End Source File
# Begin Source File

SOURCE=.\safmqman.bmp
# End Source File
# Begin Source File

SOURCE=.\SAFMQManagerRoot.rgs
# End Source File
# Begin Source File

SOURCE=.\res\server_c_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\server_dc_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\server_u_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\strip_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\strip_256_16.bmp
# End Source File
# Begin Source File

SOURCE=.\res\strip_32.bmp
# End Source File
# Begin Source File

SOURCE=.\res\user.ico
# End Source File
# Begin Source File

SOURCE=.\res\user_32.bmp
# End Source File
# End Group
# End Target
# End Project
# Section SAFMQ Manager Snapin : {00000000-0000-0000-0000-800000800000}
# 	1:15:IDD_GROUPEDITOR:121
# 	1:15:IDB_GROUPEDITOR:119
# 	1:15:IDR_GROUPEDITOR:120
# End Section
