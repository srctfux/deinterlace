@echo off
echo Dscaler make a sourceforge build batch file
echo (c) John Adcock 2008
if "%1" == "" goto usage
if "%2" == "" goto usage
cd ..\..
md DScalerBuild%1
cd DScalerBuild%1
cvs -z3 -d:ext:%2@deinterlace.cvs.sourceforge.net:/cvsroot/deinterlace co DScaler
cvs -z3 -d:ext:%2@deinterlace.cvs.sourceforge.net:/cvsroot/deinterlace co DSRend
cd DScaler\Dscaler
..\..\..\Dscaler\Release\verinc
cvs commit -m "Update Version files for release %1"
cd ..
pause
cd ..\DSRend
7z a -tzip ..\DScaler\DSRend%1src.zip *.* -r
cd ..\DScaler
7z a -tzip ..\DScaler%1src.zip *.* -r
cd Help
"c:\Program Files\HTML Help Workshop\hhc.exe" Dscaler.hhp
cd ..\DScaler
vcbuild Dscaler2005.sln "Release|Win32"
rem cd ..\Driver\DSDrvNT
rem cmd /c ..\..\Install\makeSfBuild2.bat 64
rem cmd /c ..\..\Install\makeSfBuild2.bat AMD64
cd ..\..\Install
"c:\Program Files\Inno Setup 5\Compil32.exe" /cc DScaler.iss
copy Output\Setup.exe ..\..\DScaler%1.exe
cd ..\..
del /f /q /s DSRend
rd /s /q DSRend
del /f /q /s DScaler  
rd /s /q DScaler
echo Break if there was a problem with the above build
echo Otherwise pressing enter will send the files to the
echo incoming directory on sourceforge ready to be released
pause
echo cd incoming > ftp.txt
echo bin >> ftp.txt
echo put DScaler%1.exe >> ftp.txt
echo put DScaler%1src.zip >> ftp.txt
echo bye >> ftp.txt
ftp -s:ftp.txt -A upload.sourceforge.net
del ftp.txt
goto endofbatch
:usage
echo To use this program enter the build number as parameter 1
echo and a valid sourceforge user for parameter 2
:endofbatch