# BSML UI migration pass

Added `src/UI/BSMLSettings.cpp` and `include/UI/BSMLSettings.hpp`. The implementation uses BSML 0.4.55's `BSML::Lite::CreateToggle` and `CreateIncrementSetting` declarations from the supplied headers, replacing the old QuestUI widget APIs for the core six controls.

## Important status

This is an isolated implementation pass, **not yet wired into `load()`**. `main.cpp` still uses the legacy QuestUI includes, logger and registration path, so this source is intentionally not added to the active build target. The next integration pass must: (1) switch startup to Scotland2 callbacks, (2) use Paper2 logger calls, (3) call `BSML::Init()` and register `BSMLSettings::Build` through `BSML::Register::RegisterMainMenuViewControllerMethod`, (4) remove the old QuestUI UI sources from the build, and (5) add a real QPM/CMake build target.

The image-list dropdown and refresh action are not implemented in this pass. Image loading itself remains in the original `main.cpp`. No ARM64 build or QMOD was produced.
