# Contributing

Thanks for helping keep Skyrim Legendary Edition alive.

## Reporting a bug

Open an [issue](https://github.com/Mhacks79/TrueHUD-LE/issues/new/choose) with your Skyrim LE and SKSE versions, the
TrueHUD LE version, the steps to reproduce, and whether it still happens with other UI, HUD or ENB mods disabled.
Attach `Documents\My Games\Skyrim\SKSE\TrueHUD.log` and the SKSE log when you can.

## Code

- Win32 (x86), MSVC v143, C++17. Match the style of the surrounding code.
- Engine addresses are read from `TESV.exe` 1.9.32.0.8; never copy an address from a Special Edition source or the
  Address Library. A finding is welcome when it says how it was established (calling convention and how it was
  proved, the function it was read in, tested in game or only read from the binary).
- Keep the offline harness green and add a check with every behaviour change (`tests/Harness.cpp`).
- Do not edit `upstream/`: it is an unmodified copy of the original. Never commit `TESV.exe` or any other game file.

## Licence of contributions

This project is a modified version of TrueHUD by Ersh, under **GPL-3.0-or-later with the Modding Exception and the
GPL-3.0 Linking Exception** (see `COPYING`, `EXCEPTIONS`). By contributing you agree that your work is distributed
under the same terms.
