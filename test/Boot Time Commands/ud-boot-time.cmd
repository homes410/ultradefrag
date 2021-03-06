@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

; enable debugging
call C:\ud-boot-time-test-debug.cmd

;udefrag C: -a
;udefrag C:
;udefrag C: -o
udefrag C: -q

; disable boot time scan
boot-off

; comment out next line to run test suite
exit

; Test Suite

; Allow the user to break out of script execution
echo.
echo Hit 'ESC' to enter interactive mode
echo Script execution will continue in 5 seconds
pause 5000

; Execute boot time command test suite script
; general commands test
call C:\ud-boot-time-test-suite.cmd
; udefrag test
call C:\ud-boot-time-test-udefrag.cmd
echo.
echo Test Suite finished ...
echo.
