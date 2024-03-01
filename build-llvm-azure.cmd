FOR /F "tokens=1 delims=" %%F IN ('.\scripts\vswhere.cmd') DO SET result=%%F
CALL "%result%\Common7\Tools\VsDevCmd.bat"
IF %ERRORLEVEL% GEQ 1 CALL:FAILED_CASE

echo "VS path: %result%"

CALL ".\build-llvm.cmd"
IF %ERRORLEVEL% GEQ 1 CALL:FAILED_CASE
GOTO END_CASE

:FAILED_CASE
echo "Failed to find an instance of Visual Studio. Please check it is correctly installed."
exit /B %errorlevel%
GOTO END_CASE
:END_CASE
GOTO :EOF
