
REM ********** Set path for .net framework2.0, sandcastle,hhc,hxcomp****************************

set PROJECT=safmq.net
set STYLE=safmq.net
set BSHLP=\blacksheep\help

set PATH=c:\WINDOWS\Microsoft.NET\Framework\v2.0.50727;%DXROOT%\ProductionTools;C:\Program Files\HTML Help Workshop;C:\Program Files\Microsoft Help 2.0 SDK;%PATH%

if exist output rmdir output /s /q

REM REM ********** Compile source files ****************************
REM 
REM csc /t:library /doc:comments.xml test.cs
REM ::if there are more than one file, please use [ csc /t:library /doc:comments.xml *.cs ]

REM ********** Call MRefBuilder ****************************

REM MRefBuilder test.dll /out:reflection.org

MRefBuilder %PROJECT%.dll /out:reflection.org
rem PAUSE





REM ********** Apply Transforms ****************************

rem if {%1} == {vs2005} (
XslTransform /xsl:"%DXROOT%\ProductionTransforms\ApplyVSDocModel.xsl" reflection.org /xsl:"%DXROOT%\ProductionTransforms\AddFriendlyFilenames.xsl" /out:reflection.xml /arg:IncludeAllMembersTopic=true /arg:IncludeInheritedOverloadTopics=true
rem PAUSE
rem ) else if {%1} == {hana} (
rem XslTransform /xsl:"%DXROOT%\ProductionTransforms\ApplyVSDocModel.xsl" reflection.org /xsl:"%DXROOT%\ProductionTransforms\AddFriendlyFilenames.xsl" /out:reflection.xml /arg:IncludeAllMembersTopic=false /arg:IncludeInheritedOverloadTopics=true
rem ) else (
rem XslTransform /xsl:"%DXROOT%\ProductionTransforms\ApplyPrototypeDocModel.xsl" reflection.org /xsl:"%DXROOT%\ProductionTransforms\AddGuidFilenames.xsl" /out:reflection.xml 
rem )

XslTransform /xsl:"%DXROOT%\ProductionTransforms\ReflectionToManifest.xsl"  reflection.xml /out:manifest.xml
rem PAUSE

REM call "%DXROOT%\Presentation\%1\copyOutput.bat"
REM MJB
call "%BSHLP%\Presentation\%STYLE%\copyOutput.bat"
rem PAUSE

REM ********** Call BuildAssembler ****************************
rem BuildAssembler /config:"%DXROOT%\Presentation\%1\configuration\sandcastle.config" manifest.xml
REM MJB
BuildAssembler /config:"%BSHLP%\Presentation\%STYLE%\configuration\sandcastle.config" manifest.xml
rem PAUSE

 
XslTransform /arg:project=%PROJECT% /xsl:"%DXROOT%\ProductionTransforms\ReflectionToChmProject.xsl" reflection.xml /out:Output\%PROJECT%.hhp
rem PAUSE


REM **************Generate an intermediate Toc file that simulates the Whidbey TOC format.

rem if {%1} == {prototype} (
rem XslTransform /xsl:"%DXROOT%\ProductionTransforms\createPrototypetoc.xsl" reflection.xml /out:toc.xml 
rem ) else (
XslTransform /xsl:"%DXROOT%\ProductionTransforms\createvstoc.xsl" reflection.xml /out:toc.xml 
rem PAUSE
rem )

REM ************ Generate CHM help project ******************************

XslTransform /xsl:"%DXROOT%\ProductionTransforms\TocToChmContents.xsl" toc.xml /out:Output\%PROJECT%.hhc
rem PAUSE


XslTransform /xsl:"%DXROOT%\ProductionTransforms\ReflectionToChmIndex.xsl" reflection.xml /out:Output\%PROJECT%.hhk
rem PAUSE

hhc output\%PROJECT%.hhp
rem PAUSE

cd output

cd ..


REM ************ Generate HxS help project **************************************

call "%DXROOT%\Presentation\Prototype\copyHavana.bat"
rem PAUSE

XslTransform /xsl:"%DXROOT%\ProductionTransforms\TocToHxSContents.xsl" toc.xml /out:Output\%PROJECT%.HxT
rem PAUSE

:: If you need to generate hxs, please uncomment the following line. Make sure "Microsoft Help 2.0 SDK" is installed on your machine.
::hxcomp.exe -p output\test.hxc

set PROJECT=
