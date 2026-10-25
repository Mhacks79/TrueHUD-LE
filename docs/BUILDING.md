# Building and packaging

## The plugin (DLL)

Requirements: Visual Studio / Build Tools 2022 with the MSVC v143 x86 tools and a Windows SDK. No SE, AE, VR or
CommonLibSSE-NG SDK is used.

```powershell
powershell -ExecutionPolicy Bypass -File Build.ps1
```

If MSBuild is not found automatically, pass `-MSBuild <path to MSBuild.exe>`. The script rebuilds the SKSE common
library from `sdk/common` and the Release Win32 DLL; the output is `build\TrueHUD.dll`. Generated files stay under
`build\` in this tree, never in the game folder.

### Tests

```powershell
powershell -ExecutionPolicy Bypass -File Build.ps1 -Tests
python prepare_harness_fixtures.py
$env:FAKEGAME_TIMEOUT_MS = "30000"
build\TrueHUDHarness.exe
build\TrueHUDBinaryProbe.exe (Resolve-Path build\TrueHUD.dll).Path
```

`-Tests` also builds the offline harness (a fake Skyrim inside a separate process, `harness/`) and the probe that loads
the actual DLL. Building the tests does not launch Skyrim and does not run anything. The fixtures
(`build/Data/SKSE/Plugins/TrueHUD/TrueHUD_base.ini`) are for the bench only: never install them.

## The other parts of the mod

The installable mod is the DLL plus files that are not built by `Build.ps1`:

| Part | Source | How |
|---|---|---|
| Movies (5 SWF) | `upstream/TrueHUD/swf/` (AS2 and FLA) | The original's compiled movies, with the SWF version header adapted to 10. Rebuilding them needs the Flash authoring tool |
| MCM script | `papyrus/Source/Scripts/TrueHUD_MCM.psc` | The LE Papyrus compiler, the SKSE 1.7.3 script sources and the SkyUI 5.1 LE framework sources. Its only plugin natives are `ReadSetting`, `WriteSetting` and `CommitSettings` |
| Plugin file (ESP) and SEQ | The original's plugin | Converted to LE form versions with the ESL flag cleared, and reloaded through xEdit. The SEQ keeps the start-game quest ID so the plugin's master index is preserved |
| Settings and boss rules | `src/SettingsDefaults.h`, the original's `settings.ini` | Generated from the supplied TrueHUD 1.1.10 settings |

No Bethesda master files and no extracted vanilla movies are part of this repository.

## Native LE options and diagnostics

Copy `config/TrueHUD_LE.ini` to `Data/SKSE/Plugins/TrueHUD/TrueHUD_LE.ini` in the mod's runtime package.
The native compiled defaults match that file. Logging defaults to off; `bEnableLogging=1` enables diagnostics
and bounded phase timings after restart. Visibility intervals affect ordinary-actor world occlusion only,
not live health/magicka/stamina reads. See [PERFORMANCE.txt](PERFORMANCE.txt) for limits and comparison steps.
The GitHub Actions artifact contains the DLL, this INI and licences; it is not the complete installable mod.
