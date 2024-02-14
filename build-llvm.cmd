set MY_DIR=%~dp0
set HOST=windows
set BUILD_DIR=%MY_DIR%\xa-build
set ARTIFACTS_DIR=%MY_DIR%\artifacts
set SOURCE_DIR=%MY_DIR%external\llvm\llvm

set PROJECTS=lld
set TARGETS=X86;ARM;AArch64
set BINARIES=llvm-mc.exe llvm-strip.exe lld.exe llc.exe
set PDBS=llvm-mc.pdb llvm-strip.pdb lld.pdb llc.pdb

set HOST_BUILD_DIR=%BUILD_DIR%\%HOST%
set HOST_BIN_DIR=%HOST_BUILD_DIR%\Release\bin
set HOST_ARTIFACTS_DIR=%ARTIFACTS_DIR%\%HOST%
set LLVM_VERSION_FILE=%HOST_ARTIFACTS_DIR%\llvm-version.txt
t 
set CXXFLAGS="/Qspectre /sdl /guard:cf"

if exist %HOST_BUILD_DIR% (rmdir /S /Q %HOST_BUILD_DIR%)
mkdir %HOST_BUILD_DIR%

if exist %HOST_ARTIFACTS_DIR% (rmdir /S /Q %HOST_ARTIFACTS_DIR%)
mkdir %HOST_ARTIFACTS_DIR%\bin

cd %HOST_BUILD_DIR%

cmake --version
cmake --help

cmake -G "Visual Studio 17 2022" -A x64 ^
 -DCMAKE_EXE_LINKER_FLAGS_INIT="/PROFILE /DYNAMICBASE /CETCOMPAT /guard:cf" ^
 -DBUILD_SHARED_LIBS=OFF ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded" ^
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

msbuild /p:Configuration=Release /m tools\llvm-mc\llvm-mc.vcxproj
msbuild /p:Configuration=Release /m tools\llvm-objcopy\llvm-objcopy.vcxproj
msbuild /p:Configuration=Release /m tools\lld\tools\lld\lld.vcxproj
msbuild /p:Configuration=Release /m tools\llc\llc.vcxproj

move %HOST_BIN_DIR%\llvm-objcopy.exe %HOST_BIN_DIR%\llvm-strip.exe
copy %HOST_BIN_DIR%\llvm-objcopy.pdb %HOST_BIN_DIR%\llvm-strip.pdb
for %%b in (%BINARIES%) DO (
  copy %HOST_BIN_DIR%\%%b %HOST_ARTIFACTS_DIR%\bin\%%b
)
for %%p in (%PDBS%) DO (
  copy %HOST_BIN_DIR%\%%p %HOST_ARTIFACTS_DIR%\bin\%%p
)

cd %MY_DIR%
