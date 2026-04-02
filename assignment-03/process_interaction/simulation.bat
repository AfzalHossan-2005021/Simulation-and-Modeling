@echo off
REM Compile and run mtbank.cpp on Windows (requires g++ in PATH)
g++ mtbank.cpp -o simulation.exe
if errorlevel 1 (
  echo Compilation failed.
  exit /b 1
)
simulation.exe
set exitCode=%ERRORLEVEL%
del simulation.exe
exit /b %exitCode%
