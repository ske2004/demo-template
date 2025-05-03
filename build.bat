@echo off

WHERE cl >NUL 2>&1
IF %ERRORLEVEL% NEQ 0 GOTO :NO_MSVC

WHERE crinkler >NUL 2>&1
IF %ERRORLEVEL% NEQ 0 GOTO :NO_CRINKLER

GOTO :MAIN

:NO_MSVC
ECHO Run init_msvc.bat first!
EXIT /B 1

:NO_CRINKLER
ECHO No crinkler, run init_msvc.bat, or download it from https://github.com/runestubbe/Crinkler/releases and add it to PATH.
EXIT /B 1

:MAIN
IF NOT EXIST bin MKDIR bin
ECHO ^>Bob the builder, can we build it?
cl main.c /O2 /c /GS- || EXIT /B
crinkler main.obj user32.lib kernel32.lib gdi32.lib dsound.lib dxguid.lib /SUBSYSTEM:WINDOWS /NODEFAULTLIB /out:bin\demo.exe || EXIT /B
DEL *.obj
ECHO ^>Bob the builder, yes we can! (Tutututu)
GOTO :EOF