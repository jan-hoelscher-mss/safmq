REM ********** Set path for .net framework2.0, sandcastle,hhc,hxcomp****************************

set PROJECT=safmq.net
set PROJ_PATH=..\safmq.net\safmq.net\bin\Release
set STYLE=safmq.net
set BSHLP=.\help

set TMPPATH=%PATH%
set PATH=c:\WINDOWS\Microsoft.NET\Framework\v2.0.50727;%DXROOT%\ProductionTools;C:\Program Files\HTML Help Workshop;C:\Program Files\Microsoft Help 2.0 SDK;%PATH%

if exist output rmdir output /s /q

xcopy %PROJ_PATH%\%PROJECT%.* .

MRefBuilder %PROJECT%.dll /out:reflection.org
rem pause

XslTransform /xsl:"%DXROOT%\ProductionTransforms\ApplyVSDocModel.xsl" reflection.org /xsl:"%DXROOT%\ProductionTransforms\AddFriendlyFilenames.xsl" /out:reflection.xml /arg:IncludeAllMembersTopic=true /arg:IncludeInheritedOverloadTopics=true
rem pause

XslTransform /xsl:"%DXROOT%\ProductionTransforms\ReflectionToManifest.xsl"  reflection.xml /out:manifest.xml
rem pause

call "%BSHLP%\Presentation\%STYLE%\copyOutput.bat"
rem pause

BuildAssembler /config:"%BSHLP%\Presentation\%STYLE%\configuration\sandcastle.config" manifest.xml
rem pause
 
XslTransform /arg:project=%PROJECT% /xsl:"%DXROOT%\ProductionTransforms\ReflectionToChmProject.xsl" reflection.xml /out:Output\%PROJECT%.hhp
rem pause

XslTransform /xsl:"%DXROOT%\ProductionTransforms\createvstoc.xsl" reflection.xml /out:toc.xml 
rem pause

XslTransform /xsl:"%DXROOT%\ProductionTransforms\TocToChmContents.xsl" toc.xml /out:Output\%PROJECT%.hhc
rem pause

XslTransform /xsl:"%DXROOT%\ProductionTransforms\ReflectionToChmIndex.xsl" reflection.xml /out:Output\%PROJECT%.hhk
rem pause

hhc output\%PROJECT%.hhp
rem pause

rem cd output

rem cd ..

REM ************ Generate HxS help project **************************************
call "%DXROOT%\Presentation\Prototype\copyHavana.bat"
rem pause

XslTransform /xsl:"%DXROOT%\ProductionTransforms\TocToHxSContents.xsl" toc.xml /out:Output\%PROJECT%.HxT
rem pause

:: If you need to generate hxs, please uncomment the following line. Make sure "Microsoft Help 2.0 SDK" is installed on your machine.
::hxcomp.exe -p output\test.hxc

move %PROJECT%.* Output

set PROJECT=
set STYLE=
set PATH=%TMPPATH%
