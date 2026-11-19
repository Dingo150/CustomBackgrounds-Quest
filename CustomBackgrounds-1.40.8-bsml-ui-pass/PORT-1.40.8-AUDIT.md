# Custom Backgrounds — Beat Saber 1.40.8 port audit

## Target
- Beat Saber Quest standalone `1.40.8_7379`
- Meta Quest 2 / ARM64
- MBF-installable QMOD
- `.png`, `.jpg`, `.jpeg` images
- Original 360-degree inward-facing sphere/sky material retained

## What the supplied source actually contains
- The original project is a 2021 Quest adaptation targeting Beat Saber `1.17.1`.
- It has five `MAKE_HOOK_MATCH` hooks: `SceneManager::Internal_ActiveSceneChanged`, `TrackLaneRing::Init`, `Spectrogram::Awake`, `MainCamera::Awake`, and `MenuEnvironmentManager::ShowEnvironmentType`.
- It uses QuestUI for all three settings pages and registers a QuestUI main-menu settings flow coordinator.
- It uses custom-types for the custom flow coordinator and view controllers.
- The existing renderer creates a sphere, scales it to `-800`, and applies an embedded sky material. This is the 360-degree rendering approach and should be preserved unless a 1.40.8 runtime test proves otherwise.
- Image discovery in the supplied development archive already checks the final extension case-insensitively and checks `opendir` before reading. The config defaults `hideRings` independently from `hideLasers`. These source-level fixes are present in the extracted code.
- The image loader checks file open/read, empty data, texture allocation and `ImageConversion::LoadImage`; it swaps textures only after a successful decode.

## 1.40.8 build baseline added
`qpm.shared.json` is a new modern QPM manifest using the 1.40.8-era dependency baseline:
- beatsaber-hook `^6.4.2`
- bs-cordl `4008.*`
- custom-types `^0.18.4`
- BSML `^0.4.55`
- config-utils `=2.0.3`
- Scotland2 `^0.1.6`
- Android NDK `^27.3.13750724`

These versions are based on a published Quest project configuration targeting the 1.40.8 codegen line. They are a dependency starting point, not proof that this project builds.

## Blocking migration work
1. The old `Android.mk`, `qpm.json`, and `mod.json` are legacy. The new manifest is present, but a matching modern CMake build script and generated QPM dependency tree are not present yet.
2. The UI still includes `questui/shared/QuestUI.hpp` and uses QuestUI view/controller helpers. The new manifest deliberately uses BSML as the current UI baseline, but the source has **not** yet been migrated from QuestUI to BSML. Merely changing a dependency name would break the build.
3. The five hook signatures and every referenced generated class/member still need confirmation against the actual `bs-cordl` 4008.0.0 generated headers. The supplied logcat can confirm the runtime's mod loader/hook libraries are active, but it cannot establish C++ method signatures.
4. No QPM dependency restore has been run in this environment. QPM and the Android NDK are not installed here, so no ARM64 `.so` or `.qmod` has been compiled.

## Important runtime concerns to resolve during port
- Verify `SceneManager::Internal_ActiveSceneChanged` and its `Scene` argument types against 4008 headers.
- Verify `TrackLaneRing::Init` parameter count and types; the headset log contains generic beatsaber-hook parameter-match warnings, but those warnings are not enough to attribute a mismatch to this mod.
- Verify `Spectrogram::Awake`, `MainCamera::Awake`, and `MenuEnvironmentManager::ShowEnvironmentType` exist with the expected overloads.
- Verify the old QuestUI UI flow can be replaced with BSML without losing the image list, enable toggle, rotation/resolution settings, hide-environment/rings/lights toggles.
- Verify image texture format, material shader and sphere layer/culling behaviour on device.

## Status
This is a source audit and modern dependency-manifest preparation only. It is **not** a completed port, compiled mod, or installable QMOD. Do not install this folder through MBF.

## Additional source hardening pass (2026-10-31)

A defensive runtime-safety pass was applied to `src/EnvironmentHider.cpp`:

- Check renderer arrays before iterating them.
- Check renderer pointers before accessing their GameObjects.
- Check that a menu environment manager exists before reading its previous environment type.
- Guard optional multiplayer platform, floor renderer, and lobby-avatar objects.
- Guard the gameplay environment and its MonoBehaviour before starting the light-hiding coroutine.

These changes address possible null dereferences in scene/environment transitions. They are source-level defensive fixes only; they have not been compiled or exercised in Beat Saber 1.40.8.

## Remaining port blockers

- The UI implementation still depends on the old QuestUI API. The modern manifest currently lists BSML, but the UI has not yet been migrated to BSML; this dependency mismatch must be resolved before a modern build can be considered complete.
- The five hooks in `src/main.cpp` still need signature verification against the generated `bs-cordl` headers restored for Beat Saber 1.40.8.
- A modern CMake/build script and a validated QMOD packaging step are not yet in place.
- No ARM64 `.so` or `.qmod` has been built in this environment.

## BSML UI migration preparation (2026-10-31)

Added `resources/CustomBackgroundsSettings.bsml` as the replacement layout and `UI-MIGRATION-NOTES.md` documenting the exact host bindings needed to preserve all seven existing config keys and the original settings functions. The markup is a migration scaffold only: the C++ BSML host and event/property bindings are not yet implemented or compiled because BSML 0.4.55 headers are not restored in this environment. The original QuestUI C++ files remain in place as reference and must be excluded from a future modern target.

Added `BUILD-MIGRATION-NOTES.md` to explicitly distinguish the modern QPM manifest from the old ndk-build scripts. No build or QMOD has been produced.
