$ErrorActionPreference = 'Stop'
$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

Write-Host "Custom Backgrounds - Beat Saber 1.40.8 header preparation" -ForegroundColor Cyan

# Accept either qpm.exe or the extensionless Windows executable.
$Qpm = Join-Path $ProjectRoot 'qpm.exe'
if (-not (Test-Path $Qpm)) { $Qpm = Join-Path $ProjectRoot 'qpm' }
if (-not (Test-Path $Qpm)) {
    Write-Host "QPM executable not found beside qpm.shared.json." -ForegroundColor Yellow
    Write-Host "Copy the Windows QPM executable into this folder, then run this script again."
    exit 2
}

Write-Host "Running QPM restore..."
& $Qpm restore
if ($LASTEXITCODE -ne 0) {
    Write-Host "QPM restore failed (exit code $LASTEXITCODE). No header bundle was created." -ForegroundColor Red
    exit $LASTEXITCODE
}

$Required = @('extern/bs-cordl', 'extern/beatsaber-hook', 'extern/bsml')
$Missing = @($Required | Where-Object { -not (Test-Path (Join-Path $ProjectRoot $_)) })
if ($Missing.Count -gt 0) {
    Write-Host "Restore completed, but expected dependency folders were not found:" -ForegroundColor Yellow
    $Missing | ForEach-Object { Write-Host "  $_" }
    Write-Host "Send the QPM restore output and the extern folder; the layout may differ in your QPM release."
    exit 3
}

$Bundle = Join-Path $ProjectRoot 'CustomBackgrounds-1.40.8-headers.zip'
if (Test-Path $Bundle) { Remove-Item $Bundle -Force }
Compress-Archive -Path (Join-Path $ProjectRoot 'extern'), (Join-Path $ProjectRoot 'qpm.shared.json') -DestinationPath $Bundle -CompressionLevel Optimal
Write-Host "Header bundle created: $Bundle" -ForegroundColor Green
Write-Host "Send that ZIP back here. It contains dependency headers, not personal data."
