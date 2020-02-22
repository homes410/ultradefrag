@echo off
echo Setting common environment variables...

:: UltraDefrag version
set VERSION_MAJOR=7
set VERSION_MINOR=1
set VERSION_REVISION=3

:: alpha1, beta2, RC3, etc.
:: unset for the final releases
set RELEASE_STAGE=

:: debugging facilities
:: - set to 1 to attach UltraDefrag debugger which
::   will handle application crashes in a special way
set ATTACH_DEBUGGER=1
:: - set to 1 to send crash reports via Google Analytics service
set SEND_CRASH_REPORTS=1
:: - set to 1 to enable wxWidgets asserts raising dialog boxes
::   NOTE: don't forget to recompile wxWidgets after adjustment
set ENABLE_WX_ASSERTS=0

:: 32-bit UltraDefrag debugger triggers a lot of false positive
:: detection by antivirus software, so we have to exclude it
:: entirely to reduce maintenance costs
set EXCLUDE_DEBUGGER=1

if "%EXCLUDE_DEBUGGER%" equ "1" (
    set ATTACH_DEBUGGER=0
    set SEND_CRASH_REPORTS=0
)

:: paths to development tools
set WINSDKBASE=C:\Program Files\Microsoft SDKs\Windows\v7.1
set MINGWBASE=C:\Dev\TDM-GCC-32
set MINGWx64BASE=C:\Dev\TDM-GCC-64
set WXWIDGETSDIR=C:\Dev\wxWidgets-3.1.0
set NSISDIR=C:\Program Files (x86)\NSIS
set SEVENZIP_PATH=C:\Program Files\7-Zip
set GNUWIN32_DIR=C:\Program Files (x86)\GnuWin32\bin
set CODEBLOCKS_EXE=C:\Program Files (x86)\CodeBlocks\codeblocks.exe

set SIGNTOOL="%WINSDKBASE%\Bin\signtool.exe" sign /sha1 "63dbde1be6ffeaa2683eda72c12b4ad0d510f510" /tr http://time.certum.pl /fd sha256

:: auxiliary stuff
set VERSION=%VERSION_MAJOR%,%VERSION_MINOR%,%VERSION_REVISION%,0
set VERSION2="%VERSION_MAJOR%, %VERSION_MINOR%, %VERSION_REVISION%, 0\0"
set ULTRADFGVER=%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_REVISION%
if "%RELEASE_STAGE%" neq "" (
    set UDVERSION_SUFFIX=%ULTRADFGVER%-%RELEASE_STAGE%
) else (
    set UDVERSION_SUFFIX=%ULTRADFGVER%
)
