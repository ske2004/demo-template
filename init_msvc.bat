@ECHO OFF

IF "%DEMO_DEVENV_ACTIVATED%" EQU "1" GOTO :END_VC_VARS32
IF NOT EXIST bin MKDIR bin
WHERE crinkler >NUL 2>&1
IF %ERRORLEVEL% EQU 0 GOTO :MAIN
IF NOT EXIST bin\crinkler23\Win32\crinkler.exe CALL :DOWNLOAD_CRINKLER
SET "PATH=%PATH%;bin\crinkler23\Win32"


:MAIN
SET VCVARS2022C="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2022P="C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2022E="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2019C="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2019P="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2019E="C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2017C="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2017P="C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars32.bat"
SET VCVARS2017E="C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
REM Pick MSVC version
IF EXIST %VCVARS2022C% CALL :SET_VC_VARS32 2022 %VCVARS2022C%
IF EXIST %VCVARS2022P% CALL :SET_VC_VARS32 2022 %VCVARS2022P%
IF EXIST %VCVARS2022E% CALL :SET_VC_VARS32 2022 %VCVARS2022E%
IF EXIST %VCVARS2019C% CALL :SET_VC_VARS32 2019 %VCVARS2019C%
IF EXIST %VCVARS2019P% CALL :SET_VC_VARS32 2019 %VCVARS2019P%
IF EXIST %VCVARS2019E% CALL :SET_VC_VARS32 2019 %VCVARS2019E%
IF EXIST %VCVARS2017C% CALL :SET_VC_VARS32 2017 %VCVARS2017C%
IF EXIST %VCVARS2017P% CALL :SET_VC_VARS32 2017 %VCVARS2017P%
IF EXIST %VCVARS2017E% CALL :SET_VC_VARS32 2017 %VCVARS2017E%
IF EXIST %VCVARSPATH%  GOTO :INIT_VC_VARS32
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


:SET_VC_VARS32
SET VCVARSVERSION=%1
SET VCVARSPATH=%2
GOTO :EOF

:INIT_VC_VARS32
ECHO Using MSVC %VCVARSVERSION%
CALL %VCVARSPATH%
GOTO :END_VC_VARS32

:END_VC_VARS32
SET DEMO_DEVENV_ACTIVATED=1
ECHO Dev environment activated!
GOTO :EOF