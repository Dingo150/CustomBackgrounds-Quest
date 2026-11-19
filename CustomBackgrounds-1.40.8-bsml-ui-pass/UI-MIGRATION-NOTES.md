# BSML UI migration — implementation checkpoint

## Why this file exists
The 1.40.8 dependency baseline uses BSML rather than the original 2021 QuestUI UI. The old QuestUI view-controller classes cannot simply be relinked against BSML. The replacement UI needs a BSML host object and event/property bindings.

## Preserved controls
The replacement layout in `resources/CustomBackgroundsSettings.bsml` carries forward the original settings:
- Master enable/disable toggle
- Image list and refresh action
- Rotation offset
- Target image resolution
- Hide environment
- Hide rings
- Hide lights

The existing config keys remain unchanged: `enabled`, `selectedFile`, `imageWidth`, `rotationOffset`, `hideEnvironment`, `hideRings`, and `hideLasers`. This preserves existing config files.

## Required C++ host bindings (not yet implemented)
- `enabled`: read/write `getConfig().config["enabled"]`; on change activate/deactivate `backgroundObject` and call `HideMenuEnv()`.
- `backgroundFiles`: populate from `bgDirectoryPath`, filtering `.png`, `.jpg`, `.jpeg` case-insensitively.
- `RefreshBackgroundList`: rescan the directory and refresh the bound list.
- Selecting a list item: write `selectedFile`, persist config, and call `LoadBackground(bgDirectoryPath + filename)`.
- `rotationOffset`: clamp to 0–359, persist, and rotate the sphere using the original transform convention.
- `imageWidth`: clamp to 3840–11520 and persist. Keep the warning that larger images increase load time.
- Environment toggles: persist their existing config keys.

## Important limitation
This is the BSML layout and binding contract, not a completed live UI port. The exact C++ BSML host/binding API must be implemented against the restored BSML 0.4.55 headers. Those headers are not present in this working environment, so inventing binding declarations would risk delivering source that cannot compile. The original QuestUI files are retained for reference but must not be included in the new target.
