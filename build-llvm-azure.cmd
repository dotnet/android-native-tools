FOR /F "tokens=1 delims=" %%F IN ('.\scripts\vswhere.cmd') DO SET result=%%F
2>NUL CALL "%result%\Common7\Tools\VsDevCmd.bat"
IF ERRORLEVEL 1 CALL:FAILED_CASE

CALL ".\build-llvm.cmd"
IF ERRORLEVEL 1 CALL:FAILED_CASE
GOTO END_CASE

:FAILED_CASE
echo "Failed to find an instance of Visual Studio. Please check it is correctly installed."
GOTO END_CASE
:END_CASE
GOTO :EOF
