# Compile and run mtbank.c on Windows PowerShell (requires gcc in PATH)
try {
    & gcc ../lib/simlib.c mtbank.c -o simulation.exe
} catch {
    Write-Error "Compilation failed"
    exit 1
}
if ($LASTEXITCODE -ne 0) { Write-Error "Compilation failed"; exit $LASTEXITCODE }
& .\simulation.exe
$exitCode = $LASTEXITCODE
Remove-Item .\simulation.exe -ErrorAction SilentlyContinue
exit $exitCode
