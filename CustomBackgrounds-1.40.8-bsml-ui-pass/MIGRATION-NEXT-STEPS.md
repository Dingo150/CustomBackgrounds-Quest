# Custom Backgrounds 1.40.8 — verified migration work

## Verified against the uploaded dependency headers

* Scotland2 `shared/loader.hpp`: `SetupFunc` is `void (*)(CModInfo*) noexcept`; `LoadFunc` is `void (*)() noexcept`.
* Scotland2 `shared/modloader.h`: C++ path accessors are `modloader::get_files_dir()` and `modloader::get_external_dir()` and are `noexcept`.
* Scotland2 `shared/loader.hpp`: `modloader::ModInfo` has an explicit constructor from `CModInfo const&`.
* `beatsaber-hook/shared/config/config-utils.hpp`: `Configuration` takes `modloader::ModInfo const&`; `getDataDir` accepts the same type.
* BSML `shared/BSML.hpp`: settings can be registered using `BSML::Register::RegisterSettingsMenu(name, content_key, System::Object* host, ...)`, or with a registered `HMUI::ViewController` type.
* BSML `shared/macros.hpp`: `DECLARE_BSML_PROPERTY` and options-list macros exist. These are the property accessors expected by BSML markup.

## Critical migration consequences

The old source's `ModInfo` is not Scotland2's type. Use `modloader::ModInfo` for config-utils. The loader's exported setup callback receives `CModInfo*`, not `ModInfo&`. It must be declared `extern "C"` and `noexcept`. The old `getDataDir(info)` call remains valid after constructing `modloader::ModInfo` from `*info`.

The old logger (`beatsaber-hook/shared/utils/logging.hpp`, `Logger`, `LoggerOptions`) is not the Paper2 logger. Paper2's `LoggerContext` uses fmt-style `{}` placeholders and `warn`, not the old printf-style `warning`. Every existing log call and `INSTALL_HOOK` logger argument needs a consistent Paper2 migration; it is unsafe to just change the include.

The existing BSML resource is only markup. It is not functional: there is no host implementing the referenced `enabled`, `rotationOffset`, `imageWidth`, `hideEnvironment`, `hideRings`, `hideLasers`, `backgroundFiles`, or `RefreshBackgroundList` bindings. The old C++ UI classes still include QuestUI and cannot be compiled as-is against a dependency set that only declares BSML.

## Remaining work before a real build

1. Replace the old logger with Paper2 `LoggerContext`, convert formatting, and update hook installation calls to the current beatsaber-hook API.
2. Implement a BSML host class with bound properties and callbacks; register it using the exact BSML settings registration overload. Preserve image enumeration and persist each setting through config-utils.
3. Replace QuestUI view controllers / flow coordinator with BSML view host; remove old QuestUI source files from the build.
4. Verify CMake target include/link paths against QPM-generated `extern` layout, build ARM64 with NDK r27, package metadata for Beat Saber 1.40.8, then install/test on Quest 2.

No claim of successful compilation or QMOD validity is made by this audit.
