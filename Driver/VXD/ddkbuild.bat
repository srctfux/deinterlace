call %1\bin\setenv.bat %1 free
echo %BASEDIR%
cd \source\deinterlace\driver\vxd
if "%2" == "debug" set Debug=1
if not exist Debug md Debug
if not exist Release md Release
if "%3" == "clean" nmake clean
nmake
