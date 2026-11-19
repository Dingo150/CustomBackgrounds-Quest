# Custom Backgrounds — Quest 1.40.8 port

This is a source-development package for Beat Saber Quest 1.40.8 (build 7379). It is not an installable QMOD.

## Work included
- Scotland2 `setup`/`load` entry points and mod metadata.
- Paper2 logging adapter satisfying beatsaber-hook's logger concept.
- BSML main-menu settings registration and BSML Lite controls.
- PNG/JPG/JPEG directory scanning and selection, plus a reload-selected-background action. The image list is rescanned whenever the settings view is opened.
- Legacy QuestUI source moved out of the active `src` tree.
- CMake ARM64 build entry point and a PowerShell wrapper that checks for the Android NDK.

## Still required before an installable QMOD
1. Resolve and link the QPM-generated dependency libraries for beatsaber-hook, BSML, config-utils, custom-types, bs-cordl, Scotland2 and Paper2.
2. Complete an Android NDK ARM64 compile and fix any compiler/linker errors.
3. Package the resulting `libcustombgs.so` as a QMOD.
4. Install on a Quest 2 and test startup, settings, image loading, 360 rendering, environment hiding and gameplay transitions.

No successful ARM64 build or headset test has yet been completed. Do not install this source ZIP through MBF.
