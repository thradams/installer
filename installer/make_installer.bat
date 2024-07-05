
echo off

REM Bulding uninstall

cl  /D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_DEBUG /D_WINDOWS  /D_UNICODE /DUNICODE ^
uninstall.c w32.c process.c ^
kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib ^
/link /MANIFEST:EMBED /MANIFESTINPUT:"installer.exe.manifest" /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'" 
if %errorlevel% neq 0 exit /b %errorlevel%

REM Creating zip file
cl zip.c makezip.c -o makezip.exe
if %errorlevel% neq 0 exit /b %errorlevel%

makezip.exe
if %errorlevel% neq 0 exit /b %errorlevel%

REM Compiling resource

rc /D "_UNICODE" /D "UNICODE" /l 0x0409 /nologo /fo"installer.res"  installer.rc 

if %errorlevel% neq 0 exit /b %errorlevel%

REM Compiling installer

cl  /Od /MDd /RTC1 /D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_DEBUG /D_WINDOWS /D_UNICODE ^
/DUNICODE installer.res  installer.c setup.c process.c w32.c zip.c ^
 kernel32.lib user32.lib gdi32.lib ^
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ^
 odbc32.lib ^
/link /MANIFEST:EMBED /MANIFESTINPUT:"installer.exe.manifest" /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"


REM del makezip.exe
REM del uninstall.exe
REM del files.zip
del *.pdb
del *.obj
del *.ilk

