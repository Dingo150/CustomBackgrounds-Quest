# Port status — 1 November 2026

This is a further source-integration pass, not a release.

Implemented in this pass:
- BSML Lite dropdown image picker using a stable backing vector for the lifetime of the view.
- PNG/JPG/JPEG rescan whenever the settings view is opened.
- Reload-selected-background button.
- Legacy QuestUI C++ files moved out of the active source tree.
- Hook logger adapter now supplies the five methods required by beatsaber-hook's `is_logger` concept (`info`, `debug`, `error`, `warn`, `critical`).
- CMake explicit source list and ARM64 PowerShell build entry point.
- QMOD packaging script refuses to package unless the compiled shared library exists.
- QPM shared scripts now point at the actual root scripts rather than missing `scripts/` paths.

Validation performed:
- CMake host-side configure/generate succeeded. This only validates CMake syntax; it is not an Android build.
- BSML function signatures were checked against the supplied BSML 0.4.55 headers.
- Scotland2 loader and Paper2 logger declarations were checked against the supplied headers.

Still not complete:
- QPM-generated dependency link targets/libraries have not been wired into CMake.
- No Android NDK is available in this execution environment, so no ARM64 `.so` was produced.
- No `.qmod` was produced and no Quest 2 test was performed.
- The existing 360 sphere rendering and environment-hiding behavior still require in-game verification.
