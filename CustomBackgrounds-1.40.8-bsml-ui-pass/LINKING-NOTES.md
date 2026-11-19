# QPM native dependency linking

The CMake file now discovers native `.so` and `.a` files recursively under `extern/libs`, imports them as CMake targets, and links them to `custombgs`. This avoids assuming filenames from a different QPM release.

`GET-LINKER-INFO.ps1` reports the actual files restored by QPM and creates `qpm-linker-inputs.zip` for inspection. The build must stop if `extern/libs` is absent or contains no native libraries; headers alone cannot produce an Android shared library.

This is not yet a verified Quest build. The exact QPM library set must be inspected, and the Android NDK build must succeed before a QMOD can be called installable.
