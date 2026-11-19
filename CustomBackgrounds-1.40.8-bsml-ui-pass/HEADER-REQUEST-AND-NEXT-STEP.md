# One-time dependency-header step for the 1.40.8 port

The current work environment cannot download QPM dependencies, so the generated Beat Saber 1.40.8 C++ headers are not present here. The Quest-BSML project documentation confirms that host fields and `get_` / `set_` methods can be exposed as BSML values, and host methods can be wired as BSML actions. The exact C++ parser/host types and game hook declarations still need to be checked against the restored headers.

## What the script does

`GET-1.40.8-HEADERS.ps1` runs `qpm restore` using a QPM executable placed beside `qpm.shared.json`, checks for the expected dependency folders, and creates `CustomBackgrounds-1.40.8-headers.zip` containing `extern` and `qpm.shared.json`.

It does **not** build or install the mod. It only gathers the dependency headers required to implement the BSML host and verify the game hooks safely.

## One-time action needed

1. Extract the updated development package.
2. Put the Windows QPM executable in the `Original-Project` folder, beside `qpm.shared.json` (it may be named `qpm.exe` or `qpm`).
3. Open PowerShell in that `Original-Project` folder and run: `./GET-1.40.8-HEADERS.ps1`.
4. If it succeeds, send back `CustomBackgrounds-1.40.8-headers.zip`. If it fails, send a screenshot of the PowerShell error instead.

If QPM reports a missing package or cannot reach the network, stop there and send the error; do not change dependency versions.
