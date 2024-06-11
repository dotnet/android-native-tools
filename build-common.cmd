set MY_DIR=%~dp0
set HOST=windows
set BUILD_DIR=%MY_DIR%\xa-build
set ARTIFACTS_DIR=%MY_DIR%\artifacts
set HOST_ARTIFACTS_DIR=%ARTIFACTS_DIR%\%HOST%
set HOST_ARTIFACTS_BIN_DIR=%HOST_ARTIFACTS_DIR%\bin
set CMAKE_VS_GENERATOR=Visual Studio 17 2022
set MSVC_RUNTIME_LIBRARY="MultiThreaded"
set CXXFLAGS=/Qspectre /sdl /guard:cf
set CFLAGS=/Qspectre /sdl /guard:cf

mkdir %HOST_ARTIFACTS_BIN_DIR%
