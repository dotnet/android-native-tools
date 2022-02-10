@echo off
setlocal
set BINARIES_DIRECTORY=%~dp0

set "BINARIES_DIRECTORY=" && "%BINARIES_DIRECTORY%llvm-strip.exe" %*
if not ERRORLEVEL 0 exit /b %errorlevel%
