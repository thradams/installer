
REM PIPELINE IS 
REM 1 - Build uninstall program
REM 2 - Zip all files including uninstall
REM 3 - Build installer

cl  /D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_DEBUG /D_WINDOWS  /D_UNICODE /DUNICODE uninstall.c kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib /link /MANIFEST:EMBED /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"

cl makezip.c
makezip.exe


rc /D "_UNICODE" /D "UNICODE" /l 0x0409 /nologo /fo"installer.res"  installer.rc 

cl  /Od /MDd /RTC1 /D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_DEBUG /D_WINDOWS /D_UNICODE ^
/DUNICODE installer.res  installer.c script.c kernel32.lib user32.lib gdi32.lib ^
winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ^
odbc32.lib ^
/link /MANIFEST:EMBED /MANIFESTINPUT:"installer.exe.manifest" /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"


del makezip.exe
del uninstall.exe
del files.zip
del *.pdb
del *.obj
del *.ilk

