@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

echo.
echo Enable Debugging ...
echo.

; set debuging level (NORMAL or unset, DETAILED, PARANOID)
set UD_DBGPRINT_LEVEL=DETAILED

; disable fragmentation reports
;set UD_DISABLE_REPORTS=1

; enable dry-run
;set UD_DRY_RUN=1

; set exclude filter
;set UD_EX_FILTER=temp;recycle;system volume information

; set minimum number of fragments to process a file
;set UD_FRAGMENTS_THRESHOLD=3

; set include filter
;set UD_IN_FILTER=windows;winnt;ntuser;pagefile;hiberfil

; set log file
set UD_LOG_FILE_PATH=C:\Windows\UltraDefrag\Logs\defrag_native.log

; set progress update interval
;set UD_REFRESH_INTERVAL=100

; set file size threshold (Kb, Mb, Gb, Tb, Pb, Eb)
;set UD_FILE_SIZE_THRESHOLD=600Mb

; set time limit (0y 0d 0h 0m 0s)
;set UD_TIME_LIMIT=1h
