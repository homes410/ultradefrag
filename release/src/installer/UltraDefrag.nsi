/*
 *  UltraDefrag - a powerful defragmentation tool for Windows NT.
 *  Copyright (c) 2007-2016 Dmitri Arkhangelski (dmitriar@gmail.com).
 *  Copyright (c) 2010-2013 Stefan Pendl (stefanpe@users.sourceforge.net).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
* UltraDefrag regular installer.
*/

/*
*  NOTE: The following symbols must be
*  defined through makensis command line:
*  ULTRADFGVER=<version number in form x.y.z>
*  ULTRADFGARCH=<i386 | amd64 | ia64>
*  UDVERSION_SUFFIX
*  The following symbol is accepted too,
*  but may be undefined:
*  RELEASE_STAGE
*/

!ifndef ULTRADFGVER
!error "ULTRADFGVER parameter must be specified on the command line!"
!endif

!ifndef ULTRADFGARCH
!error "ULTRADFGARCH parameter must be specified on the command line!"
!endif

!ifndef UDVERSION_SUFFIX
!error "UDVERSION_SUFFIX parameter must be specified on the command line!"
!endif

/*
 * Constants
 */

!define IDC_ARROW  32512 ; default system cursor identifier
!define IDC_HAND   32649 ; system hand cursor identifier
!define GCLP_HCURSOR -12 ; index for SetClassLong call
!define SM_CXSMICON   49 ; index for GetSystemMetrics call

!define UD_DONATION_URL "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=55FLGB3EJTDLS"
!define UD_UNINSTALL_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\UltraDefrag"
!define UD_LOG_FILE "$TEMP\UltraDefrag_Install.log"

!if ${ULTRADFGARCH} == 'i386'
!define ROOTDIR "..\.."
!else
!define ROOTDIR "..\..\.."
!endif

/*
 * Compress installer exehead with an executable compressor
 */

!packhdr temp.dat 'upx --best -q temp.dat'

/*
 * Include additional plug-in folders
 */

!addplugindir "${ROOTDIR}\src\installer\plug_ins\Plugin"
!addincludedir "${ROOTDIR}\src\installer\plug_ins\Include"

/*
 * Installer Attributes
 */

!ifdef RELEASE_STAGE
    !if ${ULTRADFGARCH} == 'amd64'
    Name "Ultra Defragmenter v${ULTRADFGVER} ${RELEASE_STAGE} (AMD64)"
    !else if ${ULTRADFGARCH} == 'ia64'
    Name "Ultra Defragmenter v${ULTRADFGVER} ${RELEASE_STAGE} (IA64)"
    !else
    Name "Ultra Defragmenter v${ULTRADFGVER} ${RELEASE_STAGE} (i386)"
    !endif
!else
    !if ${ULTRADFGARCH} == 'amd64'
    Name "Ultra Defragmenter v${ULTRADFGVER} (AMD64)"
    !else if ${ULTRADFGARCH} == 'ia64'
    Name "Ultra Defragmenter v${ULTRADFGVER} (IA64)"
    !else
    Name "Ultra Defragmenter v${ULTRADFGVER} (i386)"
    !endif
!endif

!if ${ULTRADFGARCH} == 'i386'
InstallDir "$PROGRAMFILES\UltraDefrag"
!else
InstallDir "$PROGRAMFILES64\UltraDefrag"
!endif

InstallDirRegKey HKLM ${UD_UNINSTALL_REG_KEY} "InstallLocation"

Var OldInstallDir
Var ValidDestDir
Var DonationBtn
Var BottomText
Var HWND

OutFile "ultradefrag-${UDVERSION_SUFFIX}.bin.${ULTRADFGARCH}.exe"
LicenseData "${ROOTDIR}\src\LICENSE.TXT"
ShowInstDetails show
ShowUninstDetails show

Icon "${ROOTDIR}\src\installer\udefrag-install.ico"
UninstallIcon "${ROOTDIR}\src\installer\udefrag-uninstall.ico"

XPStyle on
RequestExecutionLevel admin

InstType "Full"
InstType "Micro Edition"

/*
 * Compiler Flags
 */

AllowSkipFiles off
SetCompressor /SOLID lzma

/*
 * Version Information
 */

VIProductVersion "${ULTRADFGVER}.0"
VIAddVersionKey  "ProductName"     "Ultra Defragmenter"
VIAddVersionKey  "CompanyName"     "UltraDefrag Development Team"
VIAddVersionKey  "LegalCopyright"  "Copyright � 2007-2013 UltraDefrag Development Team"
VIAddVersionKey  "FileDescription" "Ultra Defragmenter Setup"
VIAddVersionKey  "FileVersion"     "${ULTRADFGVER}"

/*
 * Headers
 */

!include "WinMessages.nsh"
!include "WinVer.nsh"
!include "x64.nsh"
!include "MUI2.nsh"
!include "WndSubclass.nsh"
!include "${ROOTDIR}\src\installer\UltraDefrag.nsh"
!include "${ROOTDIR}\src\installer\PresetSections.nsh"

/*
 * Modern User Interface Pages
 */

!define MUI_ICON   "${ROOTDIR}\src\installer\udefrag-install.ico"
!define MUI_UNICON "${ROOTDIR}\src\installer\udefrag-uninstall.ico"

!define MUI_WELCOMEFINISHPAGE_BITMAP   "${ROOTDIR}\src\installer\WelcomePageBitmap.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${ROOTDIR}\src\installer\WelcomePageBitmap.bmp"
!define MUI_COMPONENTSPAGE_SMALLDESC

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${ROOTDIR}\src\LICENSE.TXT"
!define MUI_DIRECTORYPAGE_TEXT_TOP "Only empty folders and folders containing a previous UltraDefrag \
    installation are valid!$\nFor any other folders the $\"Next$\" button will be disabled."
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_TEXT "$(^NameDA) has been installed on your computer.$\r$\n$\r$\n\
    You can help the project to move on by a small donation. There's no need to donate \
    $$100 or more, even a small donation of $$1/$$2 might help."
!define MUI_PAGE_CUSTOMFUNCTION_PRE .onFinishPageInit
!define MUI_PAGE_CUSTOMFUNCTION_SHOW .onFinishPageShow
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

/*
 * Component Sections
 */

Section "!UltraDefrag core files (required)" SecCore

    SectionIn 1 2 RO

    ${InstallCoreFiles}

SectionEnd

SectionGroup /e "Interfaces (at least one must be selected)" SecInterfaces

Section "Boot" SecBoot

    SectionIn 1 2

    ${InstallBootFiles}

SectionEnd

Section "Console" SecConsole

    SectionIn 1 2

    ${InstallConsoleFiles}

SectionEnd

Section "GUI (Default)" SecGUI

    SectionIn 1

    ${InstallGUIFiles}

SectionEnd

SectionGroupEnd

Section "Documentation" SecHelp

    SectionIn 1

    ${InstallHelpFiles}

SectionEnd

Section "Context menu handler (requires Console)" SecShellHandler

    SectionIn 1 2

    ${InstallShellHandlerFiles}

SectionEnd

SectionGroup /e "Shortcuts (require GUI)" SecShortcuts

Section "Start Menu icon" SecStartMenuIcon

    SectionIn 1

    ${InstallStartMenuIcon}

SectionEnd

Section "Desktop icon" SecDesktopIcon

    SectionIn 1

    ${InstallDesktopIcon}

SectionEnd

Section "Quick Launch icon" SecQuickLaunchIcon

    SectionIn 1

    ${InstallQuickLaunchIcon}

SectionEnd

SectionGroupEnd

Section "Turn off usage tracking" SecUsageTracking

    ${InstallUsageTracking}

SectionEnd

; this must always be the last install section
Section "-Finalize" SecFinalize

    SectionIn 1 2 RO

    ${RemoveObsoleteFiles}

    ${WriteTheUninstaller}

    ${UpdateUninstallSizeValue}

SectionEnd

;----------------------------------------------

Section "Uninstall"

    ${DisableX64FSRedirection}

    DetailPrint "Deregister boot time defragmenter..."
    ExecWait '"$SYSDIR\bootexctrl.exe" /u /s defrag_native'
    Delete "$WINDIR\pending-boot-off"

    ${EnableX64FSRedirection}

    ${RemoveQuickLaunchIcon}
    ${RemoveDesktopIcon}
    ${RemoveStartMenuIcon}
    ${RemoveUsageTracking}
    ${RemoveShellHandlerFiles}
    ${RemoveHelpFiles}
    ${RemoveGUIFiles}
    ${RemoveConsoleFiles}
    ${RemoveBootFiles}
    ${RemoveCoreFiles}

SectionEnd

;----------------------------------------------

Function .onInit

    ; remove old log file
    Delete ${UD_LOG_FILE}
    ClearErrors

    ${CheckAdminRights}
    ${CheckWinVersion}
    ${CheckMutex}

    ${EnableX64FSRedirection}
    InitPluginsDir

    ${DisableX64FSRedirection}

    ReadRegStr $OldInstallDir HKLM ${UD_UNINSTALL_REG_KEY} "InstallLocation"
    StrCmp $OldInstallDir "" 0 +2
    StrCpy $OldInstallDir $INSTDIR

    ${CollectFromRegistry}
    ${ParseCommandLine}

    ${EnableX64FSRedirection}

FunctionEnd

Function un.onInit

    ${CheckAdminRights}
    ${CheckMutex}

FunctionEnd

;----------------------------------------------

Function .onSelChange

    ${VerifySelections}

FunctionEnd

;----------------------------------------------

Function .onVerifyInstDir

    ${CheckDestFolder}

    StrCmp $ValidDestDir "1" +2
    Abort

FunctionEnd

;----------------------------------------------

Function .onFinishPageInit

    File "/oname=$PLUGINSDIR\Paypal_US_btn_donateCC_LG.bmp" \
        "${ROOTDIR}\src\installer\Paypal_US_btn_donateCC_LG.bmp"

FunctionEnd

Function .WndProc

    ${If} $2 = ${WM_SETCURSOR}
        Push $R0
        Push $R1
        
        ${If} $1 = $DonationBtn
            System::Call "user32::LoadCursor(i 0, i ${IDC_HAND}) i.r10"
        ${Else}
            System::Call "user32::LoadCursor(i 0, i ${IDC_ARROW}) i.r10"
        ${EndIf}
        System::Call "user32::SetClassLong(i $1, i ${GCLP_HCURSOR}, i r10) i.r11"
        
        Pop $R1
        Pop $R0
    ${EndIf}
    
FunctionEnd

Function .onDonationBtnClick

    Pop $HWND
    ExecShell open ${UD_DONATION_URL}

FunctionEnd

Function .onFinishPageShow

    Push $R0
    Push $R1

    ; add donation button, centered according to the screen DPI
    System::Call "user32::GetSystemMetrics(i ${SM_CXSMICON}) i.r10"
    ${If} $R0 <= 16
        ${NSD_CreateBitmap} 180u 120u 120 47 ""
    ${ElseIf} $R0 <= 20
        ${NSD_CreateBitmap} 186u 123u 120 47 ""
    ${ElseIf} $R0 <= 24
        ${NSD_CreateBitmap} 193u 125u 120 47 ""
    ${Else}
        ${NSD_CreateBitmap} 200u 127u 120 47 ""
    ${EndIf}
    Pop $DonationBtn
    ${NSD_SetImage} $DonationBtn "$PLUGINSDIR\Paypal_US_btn_donateCC_LG.bmp" $R0
    ${NSD_OnClick} $DonationBtn .onDonationBtnClick
    
    ; add additional controls
    ${NSD_CreateLabel} 120u 175u 195u 10u "Click Finish to close this wizard."
    Pop $BottomText
    SetCtlColors $BottomText "" "${MUI_BGCOLOR}"
    
    ; set hand cursor for the donation button
    ${WndSubclass_Subclass} $DonationBtn .WndProc $R1 $R1

    ; use default arrow for everything else
    ${WndSubclass_Subclass} $BottomText .WndProc $R1 $R1
    ${WndSubclass_Subclass} $mui.FinishPage.Image .WndProc $R1 $R1
    ${WndSubclass_Subclass} $mui.FinishPage.Title .WndProc $R1 $R1
    ${WndSubclass_Subclass} $mui.FinishPage.Text .WndProc $R1 $R1
    ${WndSubclass_Subclass} $mui.FinishPage .WndProc $R1 $R1

    Pop $R1
    Pop $R0

FunctionEnd

;----------------------------------------------

Function .onInstSuccess

    ${PreserveInRegistry}

    ${RegisterInstallationFolder}

FunctionEnd

Function un.onUninstSuccess

    DetailPrint "Removing installation directory..."
    ; safe, because installation directory is predefined
    RMDir /r $INSTDIR

    DetailPrint "Cleanup registry..."
    DeleteRegKey HKLM ${UD_UNINSTALL_REG_KEY}

    ${UnRegisterInstallationFolder}

FunctionEnd

;----------------------------------------------

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore}            "Install UltraDefrag core files (required)."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecInterfaces}      "Select at least one interface to be installed."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBoot}            "The boot time interface can defragment files which are usually locked by Windows."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecConsole}         "The command line interface can be used for automation."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecGUI}             "The traditional graphical interface displays visual representation of disk clusters."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecHelp}            "Install UltraDefrag Handbook."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShellHandler}    "Defragment individual files and folders directly from Windows Explorer."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts}       "Add icons for easy access."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenuIcon}   "Add an icon to your start menu."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopIcon}     "Add an icon to your desktop."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQuickLaunchIcon} "Add an icon to your quick launch toolbar."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUsageTracking}   "Disable web based usage tracking."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;---------------------------------------------

/* EOF */
