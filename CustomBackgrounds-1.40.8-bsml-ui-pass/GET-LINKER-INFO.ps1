$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$libDir = Join-Path $root "extern/libs"
$includeDir = Join-Path $root "extern/includes"
Write-Host "Project: $root"
Write-Host "Headers present: $(Test-Path $includeDir)"
Write-Host "Library folder present: $(Test-Path $libDir)"
if (-not (Test-Path $libDir)) {
    throw "extern/libs is missing. Run qpm restore from this project folder, then run this script again."
}
$libs = @(Get-ChildItem $libDir -Recurse -File | Where-Object { $_.Extension -in @('.so', '.a') })
if ($libs.Count -eq 0) {
    throw "No .so or .a dependency libraries were found under extern/libs. Send me the output of: Get-ChildItem .\extern -Recurse -File | Select-Object FullName,Length"
}
Write-Host "Native libraries found:" -ForegroundColor Green
$libs | ForEach-Object { Write-Host (" - {0} ({1:N0} bytes)" -f $_.FullName, $_.Length) }
$out = Join-Path $root "qpm-linker-inputs.zip"
if (Test-Path $out) { Remove-Item $out -Force }
Compress-Archive -Path (Join-Path $root "qpm.shared.json"), $libDir -DestinationPath $out -CompressionLevel Optimal
Write-Host "Created: $out" -ForegroundColor Green
Write-Host "Upload qpm-linker-inputs.zip here. It contains the exact native libraries needed to verify the linker configuration."
