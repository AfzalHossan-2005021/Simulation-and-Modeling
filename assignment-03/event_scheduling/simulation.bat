@echo off
REM Compile and run mtbank.c on Windows (requires gcc in PATH)
gcc ../lib/simlib.c mtbank.c -o simulation.exe
if errorlevel 1 (
  echo Compilation failed.
  exit /b 1
)
simulation.exe
set exitCode=%ERRORLEVEL%
del simulation.exe
exit /b %exitCode%
