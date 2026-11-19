param([switch]$Clean)
$ErrorActionPreference = "Stop"
$buildDir = Join-Path $PSScriptRoot "build"
if ($Clean -and (Test-Path $buildDir)) { Remove-Item $buildDir -Recurse -Force }
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { throw "CMake was not found. Install CMake before building." }
if (-not (Test-Path (Join-Path $PSScriptRoot "extern/includes"))) { throw "QPM dependencies are missing. Run qpm restore first." }
$ndk = $env:ANDROID_NDK_HOME
if (-not $ndk) { $ndk = $env:ANDROID_NDK_ROOT }
if (-not $ndk -and (Test-Path (Join-Path $PSScriptRoot "ndkpath.txt"))) { $ndk = (Get-Content (Join-Path $PSScriptRoot "ndkpath.txt") -Raw).Trim() }
if (-not $ndk -or -not (Test-Path (Join-Path $ndk "build/cmake/android.toolchain.cmake"))) { throw "Android NDK not found. Set ANDROID_NDK_HOME to your installed NDK folder." }
$toolchain = Join-Path $ndk "build/cmake/android.toolchain.cmake"
cmake -S $PSScriptRoot -B $buildDir -DCMAKE_TOOLCHAIN_FILE=$toolchain -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }
cmake --build $buildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "Native build failed." }
