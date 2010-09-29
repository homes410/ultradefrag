@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

set UD_IN_FILTER=windows;winnt;ntuser;pagefile;hiberfil
set UD_EX_FILTER=temp;recycle;system volume information

udefrag c: -a
;udefrag c:
;udefrag c: -o

; comment out next line to run test suite
exit

; Test Suite
echo.
echo Enable Debugging ...
echo.
set UD_DBGPRINT_LEVEL=DETAILED

; Allow the user to break out of script execution
echo.
echo Hit 'ESC' to enter interactive mode
echo Script execution will continue in 5 seconds
pause 5000

; Execute boot time command test suite script
call C:\ud-boot-time-test-suite.cmd
echo.
echo Test Suite finished ...
echo.
