@echo off
::
:: This script parses command line options passed to UltraDefrag build scripts.
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

echo.
echo %~n0 ... %*
echo.

:: set to 1 to debug parameter checking
set UD_BLD_FLG_PARSER_DEBUG=0

:: set compiler constants
set UD_BLD_FLG_USE_MINGW=1
set UD_BLD_FLG_USE_MINGW64=3
set UD_BLD_FLG_USE_WINSDK=5

:: set defaults
set UD_BLD_FLG_USE_COMPILER=0
set UD_BLD_FLG_DO_INSTALL=0
set UD_BLD_FLG_ONLY_CLEANUP=0
set UD_BLD_FLG_DIPLAY_HELP=0
set UD_BLD_FLG_IS_PORTABLE=0
set UD_BLD_FLG_BUILD_X86=0
set UD_BLD_FLG_BUILD_AMD64=0
set UD_BLD_FLG_BUILD_IA64=0
set UD_BLD_FLG_BUILD_ALL=0
set UD_BLD_FLG_BUILD_PDF=0
set UD_BLD_FLG_BUILD_DEV_DOCS=0
set UD_BLD_FLG_UPDATE_TRANSLATIONS=0

:ParseArgs
if "%1" == "--use-mingw" (
    set UD_BLD_FLG_USE_COMPILER=%UD_BLD_FLG_USE_MINGW%
    set UD_BLD_FLG_BUILD_X86=1
    set UD_BLD_FLG_BUILD_AMD64=0
    set UD_BLD_FLG_BUILD_IA64=0
)
if "%1" == "--use-winsdk" (
    set UD_BLD_FLG_USE_COMPILER=%UD_BLD_FLG_USE_WINSDK%
    set UD_BLD_FLG_BUILD_X86=0
    set UD_BLD_FLG_BUILD_AMD64=0
    set UD_BLD_FLG_BUILD_IA64=0
)
if "%1" == "--use-mingw-x64" (
    set UD_BLD_FLG_USE_COMPILER=%UD_BLD_FLG_USE_MINGW64%
    set UD_BLD_FLG_BUILD_X86=0
    set UD_BLD_FLG_BUILD_AMD64=1
    set UD_BLD_FLG_BUILD_IA64=0
)
if "%1" == "--install"      set UD_BLD_FLG_DO_INSTALL=1
if "%1" == "--clean"        set UD_BLD_FLG_ONLY_CLEANUP=1
if "%1" == "--help"         set UD_BLD_FLG_DIPLAY_HELP=1
if "%1" == "--portable"     set UD_BLD_FLG_IS_PORTABLE=1
if "%1" == "--all"          set UD_BLD_FLG_BUILD_ALL=1
if "%1" == "--x86"          set UD_BLD_FLG_BUILD_X86=1
if "%1" == "--amd64"        set UD_BLD_FLG_BUILD_AMD64=1
if "%1" == "--ia64"         set UD_BLD_FLG_BUILD_IA64=1
if "%1" == "--pdf"          set UD_BLD_FLG_BUILD_PDF=1
if "%1" == "--dev"          set UD_BLD_FLG_BUILD_DEV_DOCS=1
if "%1" == "--trans"        set UD_BLD_FLG_UPDATE_TRANSLATIONS=1

shift
if not "%1" == "" goto :ParseArgs

if "%UD_BLD_FLG_BUILD_X86%%UD_BLD_FLG_BUILD_AMD64%%UD_BLD_FLG_BUILD_IA64%" == "000" (
    set UD_BLD_FLG_BUILD_AMD64=1
)

:: skip next section if not debugging command line parser
if %UD_BLD_FLG_PARSER_DEBUG% == 0 goto :EOF

:: display variables
set ud

:: clear variables
for %%V in ( UD_BLD_FLG_USE_COMPILER UD_BLD_FLG_DO_INSTALL UD_BLD_FLG_ONLY_CLEANUP UD_BLD_FLG_DIPLAY_HELP ) do set %%V=
for %%V in ( UD_BLD_FLG_IS_PORTABLE UD_BLD_FLG_BUILD_ALL UD_BLD_FLG_BUILD_PDF UD_BLD_FLG_BUILD_DEV_DOCS ) do set %%V=
for %%V in ( UD_BLD_FLG_UPDATE_TRANSLATIONS UD_BLD_FLG_BUILD_X86 UD_BLD_FLG_BUILD_AMD64 UD_BLD_FLG_BUILD_IA64 ) do set %%V=
