@echo off
echo Setting Stefan's environment variables...

:: save username, since build process overwrites it
if not defined ORIG_USERNAME set ORIG_USERNAME=%USERNAME%

set WINSDKBASE=C:\Program Files\Microsoft SDKs\Windows\v7.1
set MINGWBASE=C:\MinGW32
set MINGWx64BASE=
set WXWIDGETSDIR=C:\wxWidgets-3.0.2
set NSISDIR=C:\Program Files\NSIS
set SEVENZIP_PATH=C:\Program Files\7-Zip
set GNUWIN32_DIR=C:\Program Files\GnuWin32\bin
set CODEBLOCKS_EXE=C:\Program Files\CodeBlocks\codeblocks.exe

rem comment out next line to enable warnings to find unreachable code
goto :EOF

set CL=/Wall /wd4619 /wd4711 /wd4706
rem set LINK=/WX
