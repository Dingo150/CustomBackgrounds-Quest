$ErrorActionPreference = "Stop"
$so = Join-Path $PSScriptRoot "build/libcustombgs.so"
if (-not (Test-Path $so)) { throw "Compiled library not found at $so. Run .\build.ps1 first; a source-only package cannot be turned into a QMOD." }
$out = Join-Path $PSScriptRoot "CustomBackgrounds.qmod"
$tmp = Join-Path $PSScriptRoot "CustomBackgrounds-qmod-staging"
if (Test-Path $tmp) { Remove-Item $tmp -Recurse -Force }
New-Item -ItemType Directory -Path $tmp | Out-Null
Copy-Item $so (Join-Path $tmp "libcustombgs.so")
Copy-Item (Join-Path $PSScriptRoot "mod.json") $tmp
Copy-Item (Join-Path $PSScriptRoot "cover.png") $tmp
if (Test-Path $out) { Remove-Item $out -Force }
Compress-Archive -Path (Join-Path $tmp "*") -DestinationPath $out -CompressionLevel Optimal
Remove-Item $tmp -Recurse -Force
Write-Host "Created $out"
