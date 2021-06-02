@echo off

set QTBINPATH=C:\Qt\5.15.2\mingw81_64\bin
set QTLIBPATH=C:\Qt\Tools\mingw810_64\bin
set WIXPATH=C:\Program Files (x86)\WiX Toolset v3.11\bin
set SSLPATH=C:\Qt\Tools\OpenSSL\Win_x64\bin

setlocal EnableDelayedExpansion
del deploy /s /q
rmdir deploy /s /q
mkdir deploy
copy files.wxs deploy\
copy dialog.bmp deploy\
copy license.rtf deploy\
copy ..\builddir\jacktrip.exe deploy\
cd deploy
set PATH=%PATH%;%WIXPATH%;%QTBINPATH%
windeployqt jacktrip.exe
copy "%QTLIBPATH%\libgcc_s_seh-1.dll" .\
copy "%QTLIBPATH%\libstdc++-6.dll" .\
copy "%QTLIBPATH%\libwinpthread-1.dll" .\
copy "%SSLPATH%\libcrypto-1_1-x64.dll" .\
copy "%SSLPATH%\libssl-1_1-x64.dll" .\
.\jacktrip --test-gui
if %ERRORLEVEL% NEQ 0 (
	echo "You need to build jacktrip with gui support to build the installer."
	exit /b 1
)
rem Get our version number
for /f "tokens=*" %%a in ('.\jacktrip -v ^| findstr VERSION') do for %%b in (%%~a) do set VERSION=%%b
echo Version=%VERSION%
for /f "tokens=* delims=" %%a in (..\jacktrip.wxs.template) do (
	set line=%%a
	set line=!line:$VERSION=%VERSION%!
	echo !line! >> jacktrip.wxs
)
candle.exe jacktrip.wxs files.wxs
light.exe -ext WixUIExtension -o JackTrip.msi jacktrip.wixobj files.wixobj
endlocal
