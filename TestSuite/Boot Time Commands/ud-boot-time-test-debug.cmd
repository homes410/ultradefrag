@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

echo.
echo Enable Debugging ...
echo.

; enable dry-run
;set UD_DRY_RUN=1

; set debuging level
;set UD_DBGPRINT_LEVEL=DETAILED
set UD_DBGPRINT_LEVEL=PARANOID

; set log file
set UD_LOG_FILE_PATH=C:\Windows\UltraDefrag\Logs\defrag_native.log
