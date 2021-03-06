@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

echo.
echo Testing single file defrag ...
echo.
echo on
udefrag C:\pagefile.sys
@echo off
echo.
pause 3000

echo.
echo Testing multiple file defrag ...
echo.
echo on
udefrag C:\ud-boot-time-test-call.cmd C:\ud-boot-time-test-suite.cmd
@echo off
echo.
pause 3000

echo.
echo Testing single directory defrag ...
echo.
echo on
udefrag C:\Windows\UltraDefrag
@echo off
echo.
pause 3000

echo.
echo Testing multiple directory defrag ...
echo.
echo on
udefrag C:\Windows\system C:\Windows\SoftwareDistribution
@echo off
echo.
pause 3000

echo.
echo Testing mixed file and directory defrag ...
echo.
echo on
udefrag C:\Windows\WindowsUpdate.log C:\Windows\system
@echo off
echo.
pause 3000
