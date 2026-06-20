# Vendored: QHotkey

These files are vendored verbatim from [Skycoder42/QHotkey](https://github.com/Skycoder42/QHotkey)
(BSD-3-Clause, see `LICENSE.QHotkey`).

- **Pinned commit:** `4e3a244d87f1f7e741e1395f2ffe825f3a8ada45` (`master`)
- **Files:** `qhotkey.{h,cpp}`, `qhotkey_p.h`, `qhotkey_{mac,win,x11}.cpp`

We pin `master` rather than the last tagged release (1.5.0) because the fixes we
care about — the Wayland crash guard (#97), macOS keycode/build fixes (#66, #98),
the Windows multi-hotkey `released` fix (#86), and `QT_NO_KEYWORDS`/CMake 4.0
compatibility — are post-1.5.0 and untagged.

The library is compiled directly into `ASHIRT::HELPERS` (static, in-tree), so
`QHOTKEY_SHARED` is left undefined and `QHOTKEY_EXPORT` expands to nothing. Only
the backend matching the current OS is compiled (see `../CMakeLists.txt`); the
platform `.cpp` files have no internal `#ifdef` guards.

## Updating

Re-copy the six files from the desired upstream commit, update the pinned commit
hash above, and re-run the per-platform manual hotkey test (X11, macOS, Windows) —
global key capture cannot be verified in CI.
