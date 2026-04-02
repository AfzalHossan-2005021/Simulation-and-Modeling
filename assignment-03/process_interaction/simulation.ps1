# Compile and run mtbank.cpp on Windows PowerShell (requires g++ in PATH)
try {
    & g++ mtbank.cpp -o simulation.exe
} catch {
    Write-Error "Compilation failed"
    exit 1
}
if ($LASTEXITCODE -ne 0) { Write-Error "Compilation failed"; exit $LASTEXITCODE }
& .\simulation.exe
$exitCode = $LASTEXITCODE
Remove-Item .\simulation.exe -ErrorAction SilentlyContinue
exit $exitCode
