@echo off
setlocal
set SCRIPT_DIR=%~dp0
set LLVM_DIR=%SCRIPT_DIR%external\llvm

for %%D in (
  "%LLVM_DIR%\.ci"
  "%LLVM_DIR%\clang\test\Analysis\LifetimeSafety"
  "%LLVM_DIR%\clang\utils\analyzer"
  "%LLVM_DIR%\flang\examples\FlangOmpReport"
  "%LLVM_DIR%\lldb\test"
  "%LLVM_DIR%\lldb\tools\lldb-dap"
  "%LLVM_DIR%\llvm\docs"
  "%LLVM_DIR%\llvm\utils\git"
  "%LLVM_DIR%\llvm\utils\vscode"
  "%LLVM_DIR%\mlir\python"
  "%LLVM_DIR%\mlir\utils\mbr"
  "%LLVM_DIR%\mlir\utils\tree-sitter-mlir"
  "%LLVM_DIR%\mlir\utils\vscode"
  "%LLVM_DIR%\third-party\benchmark\tools"
) do (
  if exist "%%~D" rmdir /S /Q "%%~D"
)
endlocal
