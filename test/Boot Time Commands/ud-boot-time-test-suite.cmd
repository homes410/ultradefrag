@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

echo.
echo Testing 'call {C:\ud-boot-time-test-call.cmd}' ...
echo.
call C:\ud-boot-time-test-call.cmd
echo.
pause 3000

echo.
echo Testing 'echo on/off' and at-sign ...
echo.
echo on
; I am a comment
@echo off
echo.
pause 5000

echo.
echo Testing 'help' ...
echo.
help
echo.
pause 3000

echo.
echo Testing 'hexview {C:\fraglist.luar}' ...
echo.
hexview C:\fraglist.luar
echo.
pause 3000

echo.
echo Testing 'history' ...
echo.
history
echo.
pause 3000

echo.
echo Testing 'man' ...
echo.
man
echo.
pause 3000

echo.
echo Testing 'man {variables}' ...
echo.
man variables
echo.
pause 3000

echo.
echo Testing 'set' ...
echo.
set
echo.
pause 3000

echo.
echo Testing 'set {p}' ...
echo.
set p
echo.
pause 3000

echo.
echo Testing 'set {ZYXEL=Eureka, it works}' ...
echo.
echo Variables starting with Z before
set z
set ZYXEL=Eureka, it works
echo Variables starting with Z after
set z
echo.
pause 3000

echo.
echo Testing 'type' ...
echo.
type
echo.
pause 3000

echo.
echo Testing 'type {C:\ud-boot-time-test-call.cmd}' ...
echo.
type C:\ud-boot-time-test-call.cmd
echo.
pause 3000
