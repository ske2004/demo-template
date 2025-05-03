@ECHO OFF

IF "%DEMO_DEVENV_ACTIVATED%" EQU "1" GOTO :END_VC_VARS32
IF NOT EXIST bin MKDIR bin
WHERE crinkler >NUL 2>&1
IF %ERRORLEVEL% EQU 0 GOTO :MAIN
IF NOT EXIST bin\crinkler23\Win32\crinkler.exe CALL :DOWNLOAD_CRINKLER
SET "PATH=%PATH%;bin\crinkler23\Win32"


:MAIN
SET VCVARS2022="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2019="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2017="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
REM Pick MSVC version
IF EXIST %VCVARS2022% GOTO :INIT_VC_VARS32_2022
IF EXIST %VCVARS2019% GOTO :INIT_VC_VARS32_2019
IF EXIST %VCVARS2017% GOTO :INIT_VC_VARS32_2017
ECHO No MSVC found
EXIT /B 1


:DOWNLOAD_CRINKLER
ECHO Crinkler not found, downloading...
WHERE curl >NUL 2>&1
IF %ERRORLEVEL% NEQ 0 GOTO :NO_CURL
WHERE tar >NUL 2>&1
IF %ERRORLEVEL% NEQ 0 GOTO :NO_TAR
curl https://github.com/runestubbe/Crinkler/releases/download/v2.3/crinkler23.zip -O -J -L
MOVE crinkler23.zip bin\crinkler.zip
tar -xf bin\crinkler.zip -C bin
GOTO :EOF


:NO_CURL
ECHO Curl not found, download Crinkler manually and add it to PATH.
EXIT /B 1

:NO_TAR
ECHO Tar not found, download Crinkler manually and add it to PATH.
EXIT /B 1


:INIT_VC_VARS32_2017
ECHO Using MSVC 2017
CALL %VCVARS2017%
GOTO :END_VC_VARS32

:INIT_VC_VARS32_2019
ECHO Using MSVC 2019
CALL %VCVARS2019%
GOTO :END_VC_VARS32

:INIT_VC_VARS32_2022
ECHO Using MSVC 2022
CALL %VCVARS2022%
GOTO :END_VC_VARS32

:END_VC_VARS32
SET DEMO_DEVENV_ACTIVATED=1
ECHO Dev environment activated!
GOTO :EOF