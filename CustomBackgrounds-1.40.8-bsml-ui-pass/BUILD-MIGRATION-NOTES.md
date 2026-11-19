# Modern build migration checkpoint

The old `Android.mk`, `Application.mk`, `build.ps1`, and `buildQmod.ps1` describe the 2021 ndk-build/BMBF workflow. They are retained as historical reference and are not a valid 1.40.8 build pipeline.

The modern manifest is `qpm.shared.json`. It declares the 1.40.8-era dependencies and QMOD output name. A real build still requires QPM restore to create the generated `extern` dependency tree, then the matching QPM/CMake integration and Android NDK toolchain.

Do not run the old scripts to produce a purported 1.40.8 QMOD. They explicitly package `libbeatsaber-hook_2_3_0.so` and target package version 1.17.1.

## Current verification state
- JSON manifest parses successfully.
- Source files and BSML layout are present.
- QPM restore has not run in this environment.
- No generated `bs-cordl` headers are available here.
- No C++ compile, ARM64 shared library, or QMOD package has been produced.
