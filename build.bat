@echo off

set CRINKLER_EXE=C:\crinkler\crinkler23\Win32\crinkler.exe
if not exist %CRINKLER_EXE% (
    set CRINKLER_EXE=crinkler.exe
)

if not exist bin mkdir bin

echo ^>Bob the builder, can we build it?

cl main.c /O1 /c /GS- || exit /b
%CRINKLER_EXE% main.obj user32.lib kernel32.lib gdi32.lib dsound.lib /SUBSYSTEM:WINDOWS /NODEFAULTLIB /out:bin\demo.exe || exit /b

del *.obj

echo ^>Bob the builder, yes we can! (Tutututu)