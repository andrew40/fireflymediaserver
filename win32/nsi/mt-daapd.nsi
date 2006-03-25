; $Id$
; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "mt-daapd"
!define PRODUCT_SERVICE "Multi-Threaded DAAP Daemon"
!define /date DATEVER "%Y%m%d"
!define PRODUCT_VERSION "cvs-${DATEVER}"
!define PRODUCT_PUBLISHER "Ron Pedde"
!define PRODUCT_WEB_SITE "http://www.mt-daapd.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\mt-daapd.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"
!include "servicelib.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "..\..\admin-root\gpl-license.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"
InstallDir "$PROGRAMFILES\mt-daapd"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\Release\mt-daapd.exe"
  File "..\..\..\win32\dll\gnu_regex.dll"
  File "..\..\..\win32\dll\pthreadVC2.dll"
  File "..\..\..\win32\dll\sqlite.dll"
  File "..\..\..\win32\dll\sqlite3.dll"
  File "..\..\..\win32\dll\zlib.dll"

  SetOutPath "$INSTDIR\admin-root"
  File "..\..\admin-root\thanks.html"
  File "..\..\admin-root\status.html"
  File "..\..\admin-root\smartpopup.html"
  File "..\..\admin-root\smart.js"
  File "..\..\admin-root\smart.html"
  File "..\..\admin-root\required.gif"
  File "..\..\admin-root\playlist.js"
  File "..\..\admin-root\playlist.html"
  File "..\..\admin-root\mt-daapd.png"
  File "..\..\admin-root\mt-daapd.js"
  File "..\..\admin-root\mt-daapd.css"
  File "..\..\admin-root\linkTransparent.gif"
  File "..\..\admin-root\linkOpaque.gif"
  File "..\..\admin-root\index.html"
  File "..\..\admin-root\hdr.html"
  File "..\..\admin-root\gpl-license.txt"
  File "..\..\admin-root\gpl-license.html"
  File "..\..\admin-root\ftr.html"
  File "..\..\admin-root\feedback.html"
  File "..\..\admin-root\favicon.ico"
  File "..\..\admin-root\DAAPApplet-0.1.jar"
  File "..\..\admin-root\CREDITS"
  File "..\..\admin-root\config-update.html"
  File "..\..\admin-root\config.html"
  File "..\..\admin-root\aspl-license.txt"
  File "..\..\admin-root\aspl-license.html"
  File "..\..\admin-root\applet.html"
  SetOutPath "$INSTDIR\admin-root\lib-js"
  File "..\..\admin-root\lib-js\prototype.js"
  SetOutPath "$INSTDIR\admin-root\lib-js\script.aculo.us"
  File "..\..\admin-root\lib-js\script.aculo.us\builder.js"
  File "..\..\admin-root\lib-js\script.aculo.us\controls.js"
  File "..\..\admin-root\lib-js\script.aculo.us\dragdrop.js"
  File "..\..\admin-root\lib-js\script.aculo.us\effects.js"
  File "..\..\admin-root\lib-js\script.aculo.us\scriptaculous.js"
  File "..\..\admin-root\lib-js\script.aculo.us\slider.js"
  File "..\..\admin-root\lib-js\script.aculo.us\unittest.js"
  SetOutPath "$INSTDIR"
  File "mt-daapd-example.conf"
  SetOverwrite off
  CopyFiles "$INSTDIR\mt-daapd-example.conf" "$INSTDIR\mt-daapd.conf"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateDirectory "$SMPROGRAMS\mt-daapd"
  CreateShortCut "$SMPROGRAMS\mt-daapd\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\mt-daapd\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\mt-daapd\Debug Mode.lnk" "$INSTDIR\mt-daapd.exe" "-d9 -f"
  CreateShortCut "$SMPROGRAMS\mt-daapd\Config File.lnk" "notepad.exe" "$INSTDIR\mt-daapd.conf"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\mt-daapd.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\mt-daapd.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  ExecWait "$INSTDIR\mt-daapd.exe -i"
;  !insertmacro SERVICE "create" "${PRODUCT_SERVICE}" "path=$INSTDIR\mt-daapd.exe;autostart=1;interact=0;shortname=mt-daapd;"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
;  !undef UN
;  !define UN "un."
;  !insertmacro SERVICE "running" "${PRODUCT_SERVICE}" ""
;  IntCmp $0 0 lbl_check_install
;  !insertmacro SERVICE "stop" "${PRODUCT_SERVICE}" ""
  
;  lbl_check_install:
;  !insertmacro SERVICE "installed" "${PRODUCT_SERVICE}" ""
;  IntCmp $0 0 lbl_svc_done
;  !insertmacro SERVICE "uninstall" "${PRODUCT_SERVICE}" ""
  
;  lbl_svc_done:

  ExecWait "net stop mt-daapd"
  ExecWait "$INSTDIR\mt-daapd.exe -u"
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\gnu_regex.dll"
  Delete "$INSTDIR\pthreadVC2.dll"
  Delete "$INSTDIR\sqlite.dll"
  Delete "$INSTDIR\zlib.dll"
  Delete "$INSTDIR\mt-daapd-example.conf"
  Delete "$INSTDIR/admin-root\applet.html"
  Delete "$INSTDIR/admin-root\aspl-license.html"
  Delete "$INSTDIR/admin-root\aspl-license.txt"
  Delete "$INSTDIR/admin-root\config.html"
  Delete "$INSTDIR/admin-root\config-update.html"
  Delete "$INSTDIR/admin-root\CREDITS"
  Delete "$INSTDIR/admin-root\DAAPApplet-0.1.jar"
  Delete "$INSTDIR/admin-root\favicon.ico"
  Delete "$INSTDIR/admin-root\feedback.html"
  Delete "$INSTDIR/admin-root\ftr.html"
  Delete "$INSTDIR/admin-root\gpl-license.html"
  Delete "$INSTDIR/admin-root\gpl-license.txt"
  Delete "$INSTDIR/admin-root\hdr.html"
  Delete "$INSTDIR/admin-root\index.html"
  Delete "$INSTDIR/admin-root\linkOpaque.gif"
  Delete "$INSTDIR/admin-root\linkTransparent.gif"
  Delete "$INSTDIR/admin-root\mt-daapd.css"
  Delete "$INSTDIR/admin-root\mt-daapd.js"
  Delete "$INSTDIR/admin-root\mt-daapd.png"
  Delete "$INSTDIR/admin-root\playlist.html"
  Delete "$INSTDIR/admin-root\playlist.js"
  Delete "$INSTDIR/admin-root\required.gif"
  Delete "$INSTDIR/admin-root\smart.html"
  Delete "$INSTDIR/admin-root\smart.js"
  Delete "$INSTDIR/admin-root\smartpopup.html"
  Delete "$INSTDIR/admin-root\status.html"
  Delete "$INSTDIR/admin-root\thanks.html"
  Delete "$INSTDIR\mt-daapd.exe"

  Delete "$SMPROGRAMS\mt-daapd\Uninstall.lnk"
  Delete "$SMPROGRAMS\mt-daapd\Website.lnk"
  Delete "$SMPROGRAMS\mt-daapd\Config File.lnk"
  Delete "$SMPROGRAMS\mt-daapd\Debug Mode.lnk"

  RMDir "$SMPROGRAMS\mt-daapd"
  RMDir "$INSTDIR/admin-root"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd