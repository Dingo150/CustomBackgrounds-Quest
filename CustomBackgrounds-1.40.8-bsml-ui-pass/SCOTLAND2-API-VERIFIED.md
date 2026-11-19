# Scotland2 API verification — supplied Quest 1.40.8 dependency headers

Date: 2026-11-01

The user supplied `Scotland2-Headers.zip`, containing `scotland2/shared/modloader.h`, `scotland2/shared/loader.hpp`, and Paper2 Scotland2 headers.

## Confirmed entry-point ABI

From `scotland2/shared/loader.hpp`:

```cpp
using SetupFunc = void (*)(CModInfo* modInfo) noexcept;
using LoadFunc = void (*)() noexcept;
using LateLoadFunc = void (*)() noexcept;
using UnloadFunc = void (*)() noexcept;
```

Therefore the legacy functions in `src/main.cpp` are not ABI-compatible:

```cpp
extern "C" void setup(ModInfo& info);
extern "C" void load();
```

The required exports are `extern "C" void setup(CModInfo* modInfo) noexcept` and `extern "C" void load() noexcept` (late-load and unload are optional).

## Confirmed paths API

From `scotland2/shared/modloader.h`:

- `modloader::get_files_dir()` returns `std::filesystem::path const&`.
- `modloader::get_external_dir()` returns `std::filesystem::path const&`.
- `modloader::get_modloader_root_load_path()` returns `std::filesystem::path const&`.

The documented files directory is the app-private `/data/user/0/com.beatgames.beatsaber/files`; the external directory is `/storage/emulated/0/Android/data/com.beatgames.beatsaber/files`. Neither is automatically the legacy ModData folder. Do not silently substitute one for the user-facing background directory. The mod should use a verified external ModData path or offer a documented fallback and ensure MBF grants access.

## Confirmed logging dependency

`beatsaber-hook/shared/utils/logging.hpp` in the supplied headers includes `paper2_scotland2/shared/logger.hpp`; Paper2 logging is not the legacy global `Logger` class used by the old code. The old `Logger& getLogger()` wrapper and printf-style calls require migration to the Paper2 API. Do not retain them unchanged.

## BSML APIs confirmed

From `bsml/shared/BSML.hpp`:

- `BSML::Init()`
- `BSML::parse_and_construct(std::string_view, UnityEngine::Transform*, System::Object* host = nullptr)`
- `BSML::Register::RegisterSettingsMenu(name, System::Type*, MenuSource, bool)`
- templated `RegisterSettingsMenu<T>(name, bool)` constrained to an `HMUI::ViewController*` type
- callback overload `RegisterSettingsMenu(name, std::function<void(HMUI::ViewController*, bool, bool, bool)>, bool)`

From `bsml/shared/macros.hpp`, `DECLARE_BSML_PROPERTY(type, name)` declares a backing field and getter/setter methods for BSML binding. This establishes a valid route to bind C++ properties, but does not itself provide a host class or the actual callbacks/settings persistence.

## Migration implications

1. Change the entry-point ABI to Scotland2's `CModInfo*`/`noexcept` contract.
2. Replace legacy `ModInfo` and `getDataDir(info)` usage with the Scotland2 `CModInfo` conversion and an explicitly verified directory policy.
3. Migrate logging to Paper2.
4. Build a BSML host `HMUI::ViewController` with bound properties/actions, register it through the verified BSML API, and persist every control through config-utils.
5. Compile against the exact QPM-restored libraries and verify the generated `.so` and `.qmod` on Quest 2.

This file records verified declarations only. It does not claim that the source has compiled or that the mod is installable.
