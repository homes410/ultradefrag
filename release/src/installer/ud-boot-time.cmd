@echo off
;--------------------------------------------------------------------
;                UltraDefrag Boot Time Shell Script
;--------------------------------------------------------------------
; !!! NOTE: THIS FILE MUST BE SAVED IN UNICODE (UTF-16) ENCODING !!!
;--------------------------------------------------------------------

set UD_IN_FILTER=*windows*;*winnt*;*ntuser*;*pagefile.sys;*hiberfil.sys
set UD_EX_FILTER=*temp*;*tmp*;*.zip;*.7z;*.rar;*dllcache*;*ServicePackFiles*

udefrag c:

exit
