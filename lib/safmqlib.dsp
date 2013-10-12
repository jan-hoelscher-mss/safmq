# Microsoft Developer Studio Project File - Name="safmqlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=safmqlib - Win32 Debug Min Dependency
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "safmqlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "safmqlib.mak" CFG="safmqlib - Win32 Debug Min Dependency"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "safmqlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Debug SSL" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Release SSL" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Release Min Dependency" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Debug SSL Min Dependency" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Debug Min Dependency" (based on "Win32 (x86) Static Library")
!MESSAGE "safmqlib - Win32 Release SSL Min Dependency" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "safmqlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "safmqlib___Win32_Release"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "librel\Release"
# PROP Intermediate_Dir "librel\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../shared" /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"librel\safmq.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "safmqlib___Win32_Debug"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libdbg"
# PROP Intermediate_Dir "libdbg\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../shared" /I ".." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"libdbg\safmq.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "safmqlib___Win32_Debug_SSL"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Debug_SSL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libdbg\Debug_SSL"
# PROP Intermediate_Dir "libdbg\Debug_SSL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "l:\development" /I ".." /I "C:\dev\openssl-0.9.8\inc32\\" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "SAFMQ_SSL" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../shared" /I ".." /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "SAFMQ_SSL" /Fr /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"libdbg\safmq-ssl.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "safmqlib___Win32_Release_SSL"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Release_SSL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "librel\Release_SSL"
# PROP Intermediate_Dir "librel\Release_SSL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "l:\development" /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "SAFMQ_SSL" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../shared" /I ".." /D "_LIB" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "SAFMQ_SSL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"librel\safmq-ssl.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "safmqlib___Win32_Release_Min_Dependency"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Release_Min_Dependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "librel"
# PROP Intermediate_Dir "librel\Release_Min_Dep"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "../shared" /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../shared" /I ".." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"librel\safmq.lib"
# ADD LIB32 /nologo /out:"librel\safmq-mindep.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "safmqlib___Win32_Debug_SSL_Min_Dependency"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Debug_SSL_Min_Dependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libdbg"
# PROP Intermediate_Dir "libdbg\Debug_Min_Dep_SSL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../shared" /I ".." /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "SAFMQ_SSL" /Fr /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../shared" /I ".." /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "SAFMQ_SSL" /Fr /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"libdbg\safmq-ssl.lib"
# ADD LIB32 /nologo /out:"libdbg\safmq-ssl-mindep.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "safmqlib___Win32_Debug_Min_Dependency"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Debug_Min_Dependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libdbg\"
# PROP Intermediate_Dir "libdbg\Debug_Min_Dep"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../shared" /I ".." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../shared" /I ".." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"libdbg\safmq.lib"
# ADD LIB32 /nologo /out:"libdbg\safmq-mindep.lib"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "safmqlib___Win32_Release_SSL_Min_Dependency"
# PROP BASE Intermediate_Dir "safmqlib___Win32_Release_SSL_Min_Dependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "librel"
# PROP Intermediate_Dir "librel\Release_Min_Dep_SSL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "../shared" /I ".." /D "_LIB" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "SAFMQ_SSL" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../shared" /I ".." /D "_LIB" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "SAFMQ_SSL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"librel\safmq-ssl.lib"
# ADD LIB32 /nologo /out:"librel\safmq-ssl-mindep.lib"

!ENDIF 

# Begin Target

# Name "safmqlib - Win32 Release"
# Name "safmqlib - Win32 Debug"
# Name "safmqlib - Win32 Debug SSL"
# Name "safmqlib - Win32 Release SSL"
# Name "safmqlib - Win32 Release Min Dependency"
# Name "safmqlib - Win32 Debug SSL Min Dependency"
# Name "safmqlib - Win32 Debug Min Dependency"
# Name "safmqlib - Win32 Release SSL Min Dependency"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MessageQueue.cpp

!IF  "$(CFG)" == "safmqlib - Win32 Release"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MQConnection.cpp

!IF  "$(CFG)" == "safmqlib - Win32 Release"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MQFactory.cpp

!IF  "$(CFG)" == "safmqlib - Win32 Release"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\SSLLockingWin32.cpp

!IF  "$(CFG)" == "safmqlib - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\sslsocket.cpp

!IF  "$(CFG)" == "safmqlib - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\tcpsocket.cpp

!IF  "$(CFG)" == "safmqlib - Win32 Release"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug"

# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release Min Dependency"

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug SSL Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Debug Min Dependency"

# ADD BASE CPP /Gm-
# SUBTRACT BASE CPP /YX
# ADD CPP /Gm-
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "safmqlib - Win32 Release SSL Min Dependency"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\shared\bufstream.h
# End Source File
# Begin Source File

SOURCE=.\client_operators.h
# End Source File
# Begin Source File

SOURCE=.\MessageQueue.h
# End Source File
# Begin Source File

SOURCE=.\MQConnection.h
# End Source File
# Begin Source File

SOURCE=.\MQFactory.h
# End Source File
# Begin Source File

SOURCE=..\safmq.h
# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\socstream.h
# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\SSLLocking.h
# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\sslsocket.h
# End Source File
# Begin Source File

SOURCE=..\shared\tcpsocket\tcpsocket.h
# End Source File
# Begin Source File

SOURCE=..\shared\url\urldecode.h
# End Source File
# End Group
# End Target
# End Project
