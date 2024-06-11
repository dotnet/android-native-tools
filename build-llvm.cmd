call build-common.cmd

set SOURCE_DIR=%MY_DIR%external\llvm\llvm

set PROJECTS=lld
set TARGETS=X86;ARM;AArch64
set BINARIES=llvm-mc.exe llvm-strip.exe lld.exe llc.exe
set PDBS=llvm-mc.pdb llvm-objcopy.pdb llvm-strip.pdb lld.pdb llc.pdb

set HOST_BUILD_DIR=%BUILD_DIR%\%HOST%\llvm
set HOST_BIN_DIR=%HOST_BUILD_DIR%\Release\bin

set LLVM_VERSION_FILE=%HOST_ARTIFACTS_DIR%\llvm-version.txt
set CXXFLAGS="/Qspectre /sdl /guard:cf"

if exist %HOST_BUILD_DIR% (rmdir /S /Q %HOST_BUILD_DIR%)
mkdir %HOST_BUILD_DIR%

cd %HOST_BUILD_DIR%

cmake --version
cmake --help

cmake -G "%CMAKE_VS_GENERATOR%" -A x64 ^
 -DCMAKE_EXE_LINKER_FLAGS_INIT="/PROFILE /DYNAMICBASE /CETCOMPAT /guard:cf" ^
 -DBUILD_SHARED_LIBS=OFF ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_MSVC_RUNTIME_LIBRARY="%MSVC_RUNTIME_LIBRARY%" ^
 -DLLVM_BUILD_BENCHMARKS=OFF ^
 -DLLVM_BUILD_DOCS=OFF ^
 -DLLVM_BUILD_EXAMPLES=OFF ^
 -DLLVM_BUILD_RUNTIMES=OFF ^
 -DLLVM_BUILD_TESTS=OFF ^
 -DLLVM_BUILD_TOOLS=OFF ^
 -DLLVM_ENABLE_ASSERTIONS=OFF ^
 -DLLVM_ENABLE_BINDINGS=OFF ^
 -DLLVM_ENABLE_FFI=OFF ^
 -DLLVM_ENABLE_IDE=ON ^
 -DLLVM_ENABLE_LIBEDIT=OFF ^
 -DLLVM_ENABLE_LIBPFM=OFF ^
 -DLLVM_ENABLE_LIBXML2=OFF ^
 -DLLVM_ENABLE_PDB=ON ^
 -DLLVM_ENABLE_PROJECTS="%PROJECTS%" ^
 -DLLVM_ENABLE_TERMINFO=OFF ^
 -DLLVM_ENABLE_THREADS=OFF ^
 -DLLVM_ENABLE_ZLIB=OFF ^
 -DLLVM_ENABLE_ZSTD=ON ^
 -DLLVM_USE_STATIC_ZSTD=ON ^
 -DLLVM_INCLUDE_BENCHMARKS=OFF ^
 -DLLVM_INCLUDE_EXAMPLES=OFF ^
 -DLLVM_INCLUDE_TESTS=OFF ^
 -DLLVM_INCLUDE_TOOLS=ON ^
 -DLLVM_OPTIMIZED_TABLEGEN=ON ^
 -DLLVM_STATIC_LINK_CXX_STDLIB=ON ^
 -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" ^
 -DLLVM_USE_CRT_MINSIZEREL=MT ^
 -DLLVM_USE_CRT_RELEASE=MT ^
 %SOURCE_DIR%
IF %ERRORLEVEL% GEQ 1 EXIT /B 1

msbuild /p:Configuration=Release /m tools\llvm-mc\llvm-mc.vcxproj
IF %ERRORLEVEL% GEQ 1 EXIT /B 2

msbuild /p:Configuration=Release /m tools\llvm-objcopy\llvm-objcopy.vcxproj
IF %ERRORLEVEL% GEQ 1 EXIT /B 3

msbuild /p:Configuration=Release /m tools\lld\tools\lld\lld.vcxproj
IF %ERRORLEVEL% GEQ 1 EXIT /B 4

msbuild /p:Configuration=Release /m tools\llc\llc.vcxproj
IF %ERRORLEVEL% GEQ 1 EXIT /B 5

move %HOST_BIN_DIR%\llvm-objcopy.exe %HOST_BIN_DIR%\llvm-strip.exe
IF %ERRORLEVEL% GEQ 1 EXIT /B 6

copy %HOST_BIN_DIR%\llvm-objcopy.pdb %HOST_BIN_DIR%\llvm-strip.pdb
IF %ERRORLEVEL% GEQ 1 EXIT /B 7

for %%b in (%BINARIES%) DO (
  copy %HOST_BIN_DIR%\%%b %HOST_ARTIFACTS_BIN_DIR%\%%b
  IF %ERRORLEVEL% GEQ 1 EXIT /B 8
)
for %%p in (%PDBS%) DO (
  copy %HOST_BIN_DIR%\%%p %HOST_ARTIFACTS_BIN_DIR%\%%p
  IF %ERRORLEVEL% GEQ 1 EXIT /B 9
)

cd %MY_DIR%
