# Microsoft Developer Studio Project File - Name="safmq" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=safmq - Win32 Debug SSL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "safmq.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "safmq.mak" CFG="safmq - Win32 Debug SSL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "safmq - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "safmq - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "safmq - Win32 Debug SSL" (based on "Win32 (x86) Console Application")
!MESSAGE "safmq - Win32 Release SSL" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "safmq - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "lib" /I "shared" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D CONFIG_ENV=\"SAFMQ_CFG\" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(OutDir)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 rpcrt4.lib Rpcrt4.lib wsock32.lib kernel32.lib Advapi32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "safmq - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "lib" /I "shared" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D CONFIG_ENV=\"SAFMQ_CFG\" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(OutDir)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rpcrt4.lib Rpcrt4.lib wsock32.lib kernel32.lib Advapi32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "safmq - Win32 Debug SSL"

# PROP BASE Use_MFC 1
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "safmq___Win32_Debug_SSL"
# PROP BASE Intermediate_Dir "safmq___Win32_Debug_SSL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_SSL"
# PROP Intermediate_Dir "Debug_SSL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "client" /I "m:\dev" /I "l:\development" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D CONFIG_ENV=SAFMQ_CFG /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "lib" /I "shared" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D CONFIG_ENV=\"SAFMQ_CFG\" /D "SAFMQ_SSL" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(OutDir)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 rpcrt4.lib Rpcrt4.lib wsock32.lib Advapi32.lib kernel32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 rpcrt4.lib Rpcrt4.lib wsock32.lib kernel32.lib Advapi32.lib ssleay32.lib libeay32.lib user32.lib gdi32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "safmq - Win32 Release SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "safmq___Win32_Release_SSL"
# PROP BASE Intermediate_Dir "safmq___Win32_Release_SSL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_SSL"
# PROP Intermediate_Dir "Release_SSL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "client" /I "m:\dev" /I "l:\development" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D CONFIG_ENV=SAFMQ_CFG /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "lib" /I "shared" /D "NDEBUG" /D "SAFMQ_SSL" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D CONFIG_ENV=\"SAFMQ_CFG\" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(OutDir)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib rpcrt4.lib Rpcrt4.lib wsock32.lib Advapi32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 rpcrt4.lib Rpcrt4.lib wsock32.lib kernel32.lib Advapi32.lib ssleay32.lib libeay32.lib user32.lib gdi32.lib /nologo /subsystem:console /machine:I386

!ENDIF 

# Begin Target

# Name "safmq - Win32 Release"
# Name "safmq - Win32 Debug"
# Name "safmq - Win32 Debug SSL"
# Name "safmq - Win32 Release SSL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AddressParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Command.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectoryList.cpp
# End Source File
# Begin Source File

SOURCE=.\ForwardThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\logger.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\ntservice.cpp
# End Source File
# Begin Source File

SOURCE=.\NTSysLogger.cpp
# End Source File
# Begin Source File

SOURCE=.\param_config.cpp
# End Source File
# Begin Source File

SOURCE=.\QAccessControl.cpp
# End Source File
# Begin Source File

SOURCE=.\QManager.cpp
# End Source File
# Begin Source File

SOURCE=.\QStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ServiceThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Signal.cpp
# End Source File
# Begin Source File

SOURCE=.\SSLServerThread.cpp

!IF  "$(CFG)" == "safmq - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmq - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmq - Win32 Debug SSL"

!ELSEIF  "$(CFG)" == "safmq - Win32 Release SSL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SystemConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemDelivery.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\thdlib.cpp
# End Source File
# Begin Source File

SOURCE=.\TransactionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\utilities.cpp
# End Source File
# Begin Source File

SOURCE=.\shared\uuidgen\uuidgenwin.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AddressParser.h
# End Source File
# Begin Source File

SOURCE=.\shared\bufstream.h
# End Source File
# Begin Source File

SOURCE=.\Command.h
# End Source File
# Begin Source File

SOURCE=.\CursorCmds.h
# End Source File
# Begin Source File

SOURCE=.\DirectoryList.h
# End Source File
# Begin Source File

SOURCE=.\ForwardThread.h
# End Source File
# Begin Source File

SOURCE=.\GroupCmds.h
# End Source File
# Begin Source File

SOURCE=.\Log.h
# End Source File
# Begin Source File

SOURCE=.\logger.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\MessageCmds.h
# End Source File
# Begin Source File

SOURCE=.\Mutex.h
# End Source File
# Begin Source File

SOURCE=.\NTSysLogger.h
# End Source File
# Begin Source File

SOURCE=.\param_config.h
# End Source File
# Begin Source File

SOURCE=.\QAccessControl.h
# End Source File
# Begin Source File

SOURCE=.\QManager.h
# End Source File
# Begin Source File

SOURCE=.\QStorage.h
# End Source File
# Begin Source File

SOURCE=.\quectrl.h
# End Source File
# Begin Source File

SOURCE=.\QueueCmds.h
# End Source File
# Begin Source File

SOURCE=.\shared\randpq.h
# End Source File
# Begin Source File

SOURCE=.\shared\winnt\regutil.h
# End Source File
# Begin Source File

SOURCE=".\safmq-Signal.h"
# End Source File
# Begin Source File

SOURCE=.\safmq.h
# End Source File
# Begin Source File

SOURCE=.\safmq_defs.h
# End Source File
# Begin Source File

SOURCE=.\server_operators.h
# End Source File
# Begin Source File

SOURCE=.\ServerThread.h
# End Source File
# Begin Source File

SOURCE=.\ServiceThread.h
# End Source File
# Begin Source File

SOURCE=.\SSLServerThread.h
# End Source File
# Begin Source File

SOURCE=.\StreamCache.h
# End Source File
# Begin Source File

SOURCE=.\SystemConnection.h
# End Source File
# Begin Source File

SOURCE=.\SystemDelivery.h
# End Source File
# Begin Source File

SOURCE=.\shared\thdlib.h
# End Source File
# Begin Source File

SOURCE=.\TransactionManager.h
# End Source File
# Begin Source File

SOURCE=.\UserCmds.h
# End Source File
# Begin Source File

SOURCE=.\shared\utilities.h
# End Source File
# Begin Source File

SOURCE=.\XactCmds.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\nteventmsgs.mc

!IF  "$(CFG)" == "safmq - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=.\nteventmsgs.mc

BuildCmds= \
	mc -r "$(OutDir)" nteventmsgs.mc

"$(OutDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\nteventmsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "safmq - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=.\nteventmsgs.mc

BuildCmds= \
	mc -r "$(OutDir)" nteventmsgs.mc

"$(OutDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\nteventmsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "safmq - Win32 Debug SSL"

# Begin Custom Build
OutDir=.\Debug_SSL
InputPath=.\nteventmsgs.mc

BuildCmds= \
	mc -r "$(OutDir)" nteventmsgs.mc

"$(OutDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\nteventmsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "safmq - Win32 Release SSL"

# Begin Custom Build
OutDir=.\Release_SSL
InputPath=.\nteventmsgs.mc

BuildCmds= \
	mc -r "$(OutDir)" nteventmsgs.mc

"$(OutDir)\MSG00409.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\nteventmsgs.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\safmq.rc
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
# End Source File
# End Group
# Begin Source File

SOURCE=.\INSTALL
# End Source File
# Begin Source File

SOURCE=.\LICENSE
# End Source File
# Begin Source File

SOURCE=.\README
# End Source File
# Begin Source File

SOURCE=.\TODO
# End Source File
# End Target
# End Project
