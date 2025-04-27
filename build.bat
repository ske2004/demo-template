if not exist bin mkdir bin
cd bin
cl ../main.c /O1 /c /GS- && crinkler main.obj user32.lib kernel32.lib gdi32.lib dsound.lib /SUBSYSTEM:WINDOWS /NODEFAULTLIB
cd ..