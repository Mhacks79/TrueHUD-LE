# Notices and licences

## TrueHUD LE

TrueHUD LE is a port to Skyrim **Legendary Edition** of **TrueHUD** by **Ersh** (ersh1)
(<https://github.com/ersh1/TrueHUD>, [Nexus SE 62775](https://www.nexusmods.com/skyrimspecialedition/mods/62775),
Skyrim Special Edition), and is a **modified version** of that work.

It is distributed under the same licence as the original:

> **GPL-3.0-or-later WITH Modding Exception AND GPL-3.0 Linking Exception (with Corresponding Source)**
> Specifically, the Modded Code is Skyrim (and its variants) and the Modding Libraries include
> [SKSE](https://skse.silverlock.org/) and Commonlib (and variants).

- [`COPYING`](COPYING): the GNU General Public License, version 3.
- [`EXCEPTIONS`](EXCEPTIONS): the Modding Exception and the GPL-3.0 Linking Exception.

Both files are copied unchanged from the original repository (commit `727e051`). The exceptions let this program link
with Skyrim and with SKSE without those becoming GPL, as the original intends; every other part of this work, and any
code used with it, must obey the GPL. A binary of this port must be accompanied by the Corresponding Source, which
includes the source of the parts of the Modding Libraries it uses: that is what `sdk/` is for.

### What is the original's, what is the port's

- The design, the widgets, the ActionScript movies, the settings, the API and the behaviour are Ersh's.
- Files adapted from the original carry a header saying so (for example `src/LootIcons.cpp`,
  `src/SettingsDefaults.h`, `src/WidgetContracts.inl`, `include/TrueHUDAPI.h`).
- The Legendary Edition port (engine access read from `TESV.exe` 1.9.32.0.8, the SKSE LE implementation, the native
  movie bridge, the SkyUI MCM, the offline test harness, the tests) is by **Mhacks79**, under the licence above.
- The history of every change is this repository's history (`git log`); a summary is in [`CHANGELOG.md`](CHANGELOG.md).
- An unmodified copy of the original source is in [`upstream/TrueHUD/`](upstream/TrueHUD/).

TrueHUD LE is an independent community port. It is not affiliated with or endorsed by Ersh.

## Third-party code included

| Component | Where | Licence |
|---|---|---|
| SKSE `skse/` sources (Ian Patterson, Stephen Abel and contributors) | `sdk/skse/skse/` | The notice in [`licenses/SKSE.txt`](licenses/SKSE.txt): the Skyrim Online team is explicitly disallowed from using these files for any purpose. Keep it with any redistribution |
| SKSE `common` library (Ian Patterson) | `sdk/common/` | zlib-style, [`licenses/SKSE-common.txt`](licenses/SKSE-common.txt): altered versions must be plainly marked, and the notice may not be removed |
| GreenSock AS2 (Jack Doyle) | `upstream/TrueHUD/extern/GreenSock-AS2/` | Original 2008-2014 copyright and terms notices preserved in its files; see [`licenses/GreenSock-Notice.txt`](licenses/GreenSock-Notice.txt). Pinned at `cad14d031564f9c668309056619465721f7d4cb4` |
| The offline test harness `harness/` | `harness/` | The author's own code, under this repository's licence |

## Not included

- `TESV.exe` and every other file of the game. They are never committed (see `.gitignore`).
- Bethesda master files and extracted vanilla movies.
- The installable runtime files (converted plugin, compiled scripts, movies), distributed as a separate mod archive.
