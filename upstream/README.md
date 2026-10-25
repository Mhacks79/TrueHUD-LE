# upstream/

`TrueHUD/` is an **unmodified copy** of the original TrueHUD source, kept here so that this repository is
self-contained:

- Project: <https://github.com/ersh1/TrueHUD> by **Ersh** (ersh1), Skyrim Special Edition
  (Nexus: <https://www.nexusmods.com/skyrimspecialedition/mods/62775>)
- Commit: `727e051f5b1935c2981e858d8145752a3e688675` (1.1.10)
- `TrueHUD/COPYING` and `TrueHUD/EXCEPTIONS` are its licence (GPL-3.0-or-later with the Modding Exception and the
  GPL-3.0 Linking Exception); they are identical to the ones at the root of this repository.
- `TrueHUD/extern/GreenSock-AS2/` is the GreenSock AS2 library the original pins as a submodule, at commit
  `cad14d031564f9c668309056619465721f7d4cb4`, with its own copyright notices and terms unchanged
  (see [`../licenses/GreenSock-Notice.txt`](../licenses/GreenSock-Notice.txt)).

Why it is here: the Legendary Edition port reuses the original's ActionScript 2 movies (`swf/`, with the SWF version
header adapted to Flash 10), the widget argument contracts, the settings defaults and the loot icon policy. The
AS2/FLA sources are the corresponding source of those movies. Nothing in this directory is built by the port's
projects, and none of it is edited: improve the port in `src/`, not here.
