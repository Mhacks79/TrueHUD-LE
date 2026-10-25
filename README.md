<div align="center">

# TrueHUD LE

**A Skyrim Legendary Edition port of TrueHUD by Ersh.**
Actor info bars, boss bars, a player widget, recent loot and floating text, drawn by a native SKSE plugin.

[![Build](https://github.com/Mhacks79/TrueHUD-LE/actions/workflows/build.yml/badge.svg)](https://github.com/Mhacks79/TrueHUD-LE/actions/workflows/build.yml)
[![License: GPL-3.0-or-later with exceptions](https://img.shields.io/badge/license-GPL--3.0--or--later%20%2B%20exceptions-blue.svg)](COPYING)
![Skyrim LE 1.9.32.0.8](https://img.shields.io/badge/Skyrim-LE%201.9.32.0.8-5ab4f0)
![SKSE 1.7.3](https://img.shields.io/badge/SKSE-1.7.3-5ab4f0)
![x86](https://img.shields.io/badge/arch-x86-lightgrey)

[Original TrueHUD on GitHub](https://github.com/ersh1/TrueHUD) ·
[Original on Nexus](https://www.nexusmods.com/skyrimspecialedition/mods/62775) ·
[Status](docs/STATUS.md) ·
[Support on Ko-fi](https://ko-fi.com/mhacks79)

</div>

---

TrueHUD LE brings [**TrueHUD**](https://github.com/ersh1/TrueHUD) by **Ersh** (ersh1), a Skyrim **Special Edition**
mod, to the original **Skyrim Legendary Edition** (`TESV.exe` 1.9.32.0.8, SKSE 1.7.3, 32-bit). It is a native port:
no Special Edition, Anniversary Edition or CommonLibSSE-NG code runs here, and every engine address was established
from the Legendary Edition executable.

> This repository holds the **source** of the port. The version described here is **TrueHUD LE 1.1.10, Test 5**,
> a development build: see [`docs/STATUS.md`](docs/STATUS.md) for exactly what has and has not been checked.
> TrueHUD LE is **not** claimed to have complete feature parity with the Special Edition mod.

## What it draws

As in the original, the HUD is made of widgets that follow the actors and the game:

- **Actor info bars**: health (and optionally magicka and stamina) above actors, for the target and for nearby actors.
- **Boss bars**: a large bar for bosses, with boss recognition rules read from INI files.
- **Player widget**: the player's resource bars, with the shout indicator and special resources.
- **Recent loot**: a list of what you pick up, with icons by item type.
- **Floating text**: damage and healing numbers.
- A **public API** (`include/TrueHUDAPI.h`) for other plugins: interface versions V1 to V4, widget loading, target and
  special resource ownership.
- A **MCM** (SkyUI) for the settings, with six pages: general, actor info bars, boss bars, player widget, recent loot
  and colours.

## Requirements

- Skyrim Legendary Edition **1.9.32.0.8** and [SKSE](https://skse.silverlock.org/) **1.7.3**
- [SkyUI](https://www.nexusmods.com/skyrim/mods/3863) 5.1 for the MCM

## Installation

The plugin needs more than the DLL: a converted plugin file, compiled scripts, the five movies and the settings. Those
runtime files are **not** in this source repository (see [`docs/STATUS.md`](docs/STATUS.md)); install TrueHUD LE from
its mod archive, not from this repository. [`docs/BUILDING.md`](docs/BUILDING.md) describes how the DLL is built and
how the other assets relate to the sources.

## Building

Requirements: Visual Studio 2022 or its Build Tools (MSVC v143 x86 tools and a Windows SDK). Python is only needed for
the test fixtures.

```powershell
powershell -ExecutionPolicy Bypass -File Build.ps1          # the DLL: build\TrueHUD.dll
powershell -ExecutionPolicy Bypass -File Build.ps1 -Tests   # plus the offline harness and the API probe
```

[GitHub Actions](.github/workflows/build.yml) does the same on every push, checks that the DLL is x86 and exports
`SKSEPlugin_Query`, `SKSEPlugin_Load` and `RequestPluginAPI`, and runs the offline harness.

## Repository layout

| Path | Content |
|---|---|
| `src/` | The plugin: engine access (`Engine.cpp`), state and sampling (`State.cpp`), the movie bridge (`UI.cpp`), settings, damage and loot signals, boss rules, the public API (`PublicAPI.cpp`) |
| `include/TrueHUDAPI.h` | The public API header for other plugins |
| `papyrus/Source/Scripts/` | `TrueHUD_MCM.psc`, the SkyUI MCM script |
| `tests/`, `harness/` | The offline fake-game harness and the actual-DLL API probe |
| `sdk/` | The SKSE LE SDK subset the build uses, with its licences |
| `upstream/TrueHUD/` | An unmodified copy of Ersh's TrueHUD 1.1.10, including its ActionScript 2 and FLA sources, see [`upstream/README.md`](upstream/README.md) |
| `licenses/` | Licence texts of the third-party code |
| `docs/` | Status, build notes and the recorded validation |

## Licence

TrueHUD LE is a modified version of TrueHUD and keeps its licence:
**[GPL-3.0-or-later](COPYING) WITH the [Modding Exception and the GPL-3.0 Linking Exception (with Corresponding Source)](EXCEPTIONS)**,
where the Modded Code is Skyrim (and its variants) and the Modding Libraries include SKSE and CommonLib (and variants).
The texts are the original's, unchanged. Third-party components keep their own licences, see [`NOTICE.md`](NOTICE.md).

## Credits

- **Ersh** (ersh1): [TrueHUD](https://github.com/ersh1/TrueHUD), the mod this is ported from, and the author of the
  widgets, the movies, the design and the API that this port follows.
- **Mhacks79**: the Legendary Edition port.
- **Ian Patterson, Stephen Abel and the SKSE team**: SKSE.
- **Jack Doyle / GreenSock**: the GreenSock AS2 tweening library used by the original movies.

TrueHUD LE is an independent community port. It is not affiliated with or endorsed by Ersh.

<div align="center">

[![Support me on Ko-fi](https://storage.ko-fi.com/cdn/brandasset/v2/support_me_on_kofi_blue.png)](https://ko-fi.com/mhacks79)

</div>
