@echo off
::
:: Build script for wxWidgets library.
:: Copyright (c) 2007-2019 Dmitri Arkhangelski (dmitriar@gmail.com).
:: Copyright (c) 2010-2013 Stefan Pendl (stefanpe@users.sourceforge.net).
::
:: This program is free software; you can redistribute it and/or modify
:: it under the terms of the GNU General Public License as published by
:: the Free Software Foundation; either version 2 of the License, or
:: (at your option) any later version.
::
:: This program is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
:: GNU General Public License for more details.
::
:: You should have received a copy of the GNU General Public License
:: along with this program; if not, write to the Free Software
:: Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
::

call ParseCommandLine.cmd %*

:: handle help request
if %UD_BLD_FLG_DIPLAY_HELP% equ 1 (
    call :usage
    exit /B 0
)

title Build started

:: set environment
call setvars.cmd
if exist "setvars_%COMPUTERNAME%_%ORIG_USERNAME%.cmd" call "setvars_%COMPUTERNAME%_%ORIG_USERNAME%.cmd"
if exist "setvars_%COMPUTERNAME%_%USERNAME%.cmd" call "setvars_%COMPUTERNAME%_%USERNAME%.cmd"
echo.

:: configure wxWidgets
copy /Y .\tools\patch\wx\setup.h "%WXWIDGETSDIR%\include\wx\msw\setup.h"

:: patch wxWidgets
copy /Y .\tools\patch\wx\debughlp.h "%WXWIDGETSDIR%\include\wx\msw\debughlp.h"
copy /Y .\tools\patch\wx\filefn.cpp "%WXWIDGETSDIR%\src\common\filefn.cpp"
copy /Y .\tools\patch\wx\languageinfo.cpp "%WXWIDGETSDIR%\src\common\languageinfo.cpp"
copy /Y .\tools\patch\wx\menu.cpp "%WXWIDGETSDIR%\src\msw\menu.cpp"
copy /Y .\tools\patch\wx\statusbar.cpp "%WXWIDGETSDIR%\src\msw\statusbar.cpp"

:: build wxWidgets
if %UD_BLD_FLG_USE_COMPILER% equ 0 (
    echo No compiler specified, using Windows SDK.
    echo.
    goto winsdk_build
)

if %UD_BLD_FLG_USE_COMPILER% equ %UD_BLD_FLG_USE_MINGW%   goto mingw_build
if %UD_BLD_FLG_USE_COMPILER% equ %UD_BLD_FLG_USE_MINGW64% goto mingw_x64_build
if %UD_BLD_FLG_USE_COMPILER% equ %UD_BLD_FLG_USE_WINSDK%  goto winsdk_build

:winsdk_build

    set BUILD_ENV=winsdk
    set OLD_PATH=%path%

    if %UD_BLD_FLG_BUILD_X86% neq 0 (
        echo --------- x86 target ---------
        set AMD64=
        set IA64=
        pushd ..
        call "%WINSDKBASE%\bin\SetEnv.Cmd" /Release /x86 /xp
        popd
        call :build_library X86 || goto fail
    )
    
    set path=%OLD_PATH%

    if %UD_BLD_FLG_BUILD_AMD64% neq 0 (
        echo --------- x64 target ---------
        set AMD64=1
        set IA64=
        pushd ..
        call "%WINSDKBASE%\bin\SetEnv.Cmd" /Release /x64 /xp
        popd
        call :build_library amd64 || goto fail
    )
    
    set path=%OLD_PATH%

    if %UD_BLD_FLG_BUILD_IA64% neq 0 (
        echo --------- ia64 target ---------
        set AMD64=
        set IA64=1
        pushd ..
        call "%WINSDKBASE%\bin\SetEnv.Cmd" /Release /ia64 /xp
        popd
        set BUILD_DEFAULT=-nmake -i -g -P
        call :build_library ia64 || goto fail
    )
    
goto success


:mingw_x64_build

    set OLD_PATH=%path%

    echo --------- x64 target ---------
    echo.
    set AMD64=1
    set IA64=
    set path=%MINGWx64BASE%\bin;%path%
    set BUILD_ENV=mingw_x64
    call :build_library amd64 || goto fail

goto success


:mingw_build

    set OLD_PATH=%path%

    echo --------- x86 target ---------
    echo.
    set AMD64=
    set IA64=
    set path=%MINGWBASE%\bin;%path%
    set BUILD_ENV=mingw
    call :build_library X86 || goto fail

goto success


:success
    echo Build success!
    title Build success!

    :: get rid of annoying dark
    :: green color left by WinSDK
    color
    
    set path=%OLD_PATH%
    set OLD_PATH=

exit /B 0

:fail
    echo Build error (code %ERRORLEVEL%)!
    title Build error (code %ERRORLEVEL%)!

    color
    
    set path=%OLD_PATH%
    set OLD_PATH=

exit /B 1

:: Builds wxWidgets library
:: Example: call :build_library X86
:build_library
    set WX_CONFIG=%BUILD_ENV%-%1

    set WX_OPTIONS=BUILD=release UNICODE=1

    :: speed compilation up
    set WX_OPTIONS=%WX_OPTIONS% USE_HTML=0 USE_WEBVIEW=0
    set WX_OPTIONS=%WX_OPTIONS% USE_MEDIA=0 USE_XRC=0
    set WX_OPTIONS=%WX_OPTIONS% USE_AUI=0 USE_RIBBON=0
    set WX_OPTIONS=%WX_OPTIONS% USE_PROPGRID=0 USE_RICHTEXT=0
    set WX_OPTIONS=%WX_OPTIONS% USE_STC=0 USE_OPENGL=0 USE_CAIRO=0
    
    echo wxWidgets compilation started...
    echo.
    pushd "%WXWIDGETSDIR%\build\msw"

    if %WindowsSDKVersionOverride%x neq v7.1x goto NoWin7SDK
    if x%CommandPromptType% neq xCross goto NoWin7SDK
    set path=%PATH%;%VS100COMNTOOLS%\..\..\VC\Bin

    :NoWin7SDK
    if %UD_BLD_FLG_ONLY_CLEANUP% equ 1 goto cleanup
    
    set WX_SDK_LIBPATH=%WXWIDGETSDIR%\lib\vc_lib%WX_CONFIG%
    if %1 equ amd64 set WX_SDK_LIBPATH=%WXWIDGETSDIR%\lib\vc_x64_lib%WX_CONFIG%
    if %1 equ ia64 set WX_SDK_LIBPATH=%WXWIDGETSDIR%\lib\vc_ia64_lib%WX_CONFIG%
    
    set WX_SDK_CPPFLAGS=/MT
    
    :: ia64 targeting SDK compiler doesn't support optimization
    if %1 equ ia64 set WX_SDK_CPPFLAGS=%WX_SDK_CPPFLAGS% /Od
    
    :: x64 optimization leads to crashes
    :: http://support.microsoft.com/en-us/kb/2280741
    if %1 equ amd64 set WX_SDK_CPPFLAGS=%WX_SDK_CPPFLAGS% /Od
    
    set WX_GCC_CPPFLAGS=-g0

    if %1 equ amd64 set WX_GCC_CPPFLAGS=%WX_GCC_CPPFLAGS% -m64
    
    if "%ENABLE_WX_ASSERTS%" equ "1" (
        set WX_SDK_CPPFLAGS=%WX_SDK_CPPFLAGS% /DENABLE_WX_ASSERTS
        set WX_GCC_CPPFLAGS=%WX_GCC_CPPFLAGS% -DENABLE_WX_ASSERTS
    )
    
    if %BUILD_ENV% equ winsdk (
        copy /Y "%WXWIDGETSDIR%\include\wx\msw\setup.h" "%WX_SDK_LIBPATH%\mswu\wx\"
        nmake -f makefile.vc TARGET_CPU=%1 %WX_OPTIONS% CFG=%WX_CONFIG% CPPFLAGS="%WX_SDK_CPPFLAGS%" || goto build_failure
    )
    if %BUILD_ENV% equ mingw (
        copy /Y "%WXWIDGETSDIR%\include\wx\msw\setup.h" "%WXWIDGETSDIR%\lib\gcc_lib%WX_CONFIG%\mswu\wx\"
        mingw32-make -f makefile.gcc %WX_OPTIONS% CFG=%WX_CONFIG% CPPFLAGS="%WX_GCC_CPPFLAGS%" || goto build_failure
    )
    if %BUILD_ENV% equ mingw_x64 (
        copy /Y "%WXWIDGETSDIR%\include\wx\msw\setup.h" "%WXWIDGETSDIR%\lib\gcc_lib%WX_CONFIG%\mswu\wx\"
        mingw32-make -f makefile.gcc %WX_OPTIONS% CFG=%WX_CONFIG% CPPFLAGS="%WX_GCC_CPPFLAGS%" || goto build_failure
    )
    goto success
    
    :cleanup
    if %BUILD_ENV% equ winsdk (
        nmake -f makefile.vc TARGET_CPU=%1 %WX_OPTIONS% CFG=%WX_CONFIG% clean || goto build_failure
    )
    if %BUILD_ENV% equ mingw (
        mingw32-make -f makefile.gcc %WX_OPTIONS% CFG=%WX_CONFIG% clean || goto build_failure
    )
    if %BUILD_ENV% equ mingw_x64 (
        mingw32-make -f makefile.gcc %WX_OPTIONS% CFG=%WX_CONFIG% clean || goto build_failure
    )
    goto success
    
    :build_failure
    popd
    set WX_CONFIG=
    set WX_OPTIONS=
    set WX_SDK_CPPFLAGS=
    set WX_SDK_LIBPATH=
    exit /B 1
    
    :success
    popd
    echo wxWidgets compilation completed successfully!
    echo.

    set WX_CONFIG=
    set WX_OPTIONS=
    set WX_SDK_CPPFLAGS=
    set WX_SDK_LIBPATH=

exit /B 0

rem Displays usage information.
:usage
    echo Usage: wxbuild [options]
    echo.
    echo Common options:
    echo --clean         perform full cleanup instead of the build
    echo.
    echo Compiler:
    echo --use-winsdk    (default)
    echo --use-mingw
    echo --use-mingw-x64 (experimental, produces incorrect code)
    echo.
    echo Target architecture (must always be after the compiler):
    echo --x86           build 32-bit binaries
    echo --amd64         build x64 binaries
    echo --ia64          build IA-64 binaries
    echo.
    echo Example:
    echo wxbuild --use-winsdk --amd64 --ia64 --clean
    echo.
    echo In this case the x64 and IA-64 winsdk builds will be cleaned up.
    echo.
    echo Without parameters the wxbuild command uses Windows SDK
    echo to compile x64 libraries.
goto :EOF
