@echo off

set GOT_REVISION=%1%
set MY_DIR=%CD%



cd bin\Win32 || goto error
del /f /q openmpt-win32.tar
del /f /q openmpt-win32-r%GOT_REVISION%.7z
copy /y ..\..\LICENSE .\ || goto error
"C:\Program Files\7-Zip\7z.exe" a -t7z -mx=9 openmpt-win32-r%GOT_REVISION%.7z mptrack.exe OpenMPT_SoundTouch_f32.dll "MIDI Input Output.dll" LICENSE || goto error
"C:\Program Files\7-Zip\7z.exe" a -ttar openmpt-win32.tar openmpt-win32-r%GOT_REVISION%.7z || goto error
del /f /q openmpt-win32-r%GOT_REVISION%.7z
cd ..\.. || goto error



goto noerror

:error
cd "%MY_DIR%"
exit 1

:noerror
cd "%MY_DIR%"
exit 0
