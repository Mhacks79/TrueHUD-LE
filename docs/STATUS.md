# Status - TrueHUD LE 1.1.10 (Test 5)

**Target.** Skyrim LE 1.9.32.0.8, SKSE 1.7.3, MSVC v143 Win32. A native port of Ersh's TrueHUD 1.1.10 (SE,
GPL-3.0-or-later with exceptions). No SE, AE, VR or CommonLibSSE-NG SDK is used: the engine access is built on the SKSE
LE SDK in `sdk/`, and the engine addresses were established from `TESV.exe`.

## What was checked

Recorded in [`SOURCE-VALIDATION.json`](SOURCE-VALIDATION.json), for the exact native sources of Test 5:

| Check | Result |
|---|---|
| Offline harness (a fake Skyrim in a separate process, see `harness/` and `tests/Harness.cpp`) | 2619 checks, 0 failures |
| Public API probe against the actual DLL (`tests/BinaryProbe.cpp`) | 15 checks, 0 failures |
| Late-exit check | 1 check, 0 failures |
| Independent rebuild of the Test 5 sources | done before delivery; the SHA-256 of the rebuilt DLL is in the JSON |

The same checks ran against these sources in the original workspace. This repository **re-lays out** the files
(`src/`, `tests/`, `sdk/`, `harness/` at the top level instead of nested folders); the first build in GitHub Actions
after that change is the check that the new layout builds. Look at the latest [Build](https://github.com/Mhacks79/TrueHUD-LE/actions/workflows/build.yml)
run before trusting a checkout.

## Reported in game

The author reports that Test 5 appears to work. That is user feedback, **not** an automated game acceptance test and
**not** a claim of complete feature parity with the Special Edition mod.

## Not verified, and known limits

- Complete SE feature parity is not claimed. Features that depend on Special Edition only mods or behaviour are not
  covered.
- The offline harness exercises the plugin against a fake game. It does not replace testing in Skyrim.
- The plugin writes `Documents\My Games\Skyrim\SKSE\TrueHUD.log` in this development build (logging is not compiled out
  as it is in the author's other releases).
- The five movies were converted from the original's compiled movies by adapting the SWF version header to 10. Their
  AS2/FLA sources are in `upstream/TrueHUD/swf/`. Rebuilding them needs the Flash authoring tool, which is not bundled.
- The converted plugin file, compiled scripts and movies are **not** in this repository.
