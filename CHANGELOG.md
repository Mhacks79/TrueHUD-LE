# Changelog

## 1.1.10 Test 7 - 2026-10-25

- Configure the standalone shout indicator when it is created or its opacity changes, instead of rebuilding its
  animation timelines every frame while boss bars hide the compass.
- Skip successful unchanged boss, actor, player, loot and shout transforms and unchanged main/custom depth sorts;
  retry failed calls and update changed layout, visibility and depth immediately.
- Keep unrelated actor color changes from reconfiguring every other widget; preserve override/revert behavior.
- Keep every live resource and shout progress/keepalive update. Movie bodies and visual quality are unchanged.
- Include Test 6 native crowd improvements: staggered bounded LOS refreshes, one steady sampling owner, a shared
  settings snapshot, disabled-feature sampling skip, and event-only loot resolution.
- Logging is off by default. Optional bounded timings separate sampling, UI work, movie advance and render.
- Add populated boss/shout/custom-widget regressions. Workshop validation: 2601 checks, late-exit callback, 15 actual
  DLL API checks and 76 compiled bug-restoration mutants. Repository layout is rebuilt separately; no game FPS claim.

## TrueHUD LE 1.1.10, Test 5 - 2026-10-25 (source published)

Native Skyrim Legendary Edition port of TrueHUD 1.1.10 by Ersh.

- Actor info bars, boss bars, player widget, recent loot and floating text driven by a native SKSE LE plugin.
- Public API for other plugins (interface versions V1 to V4), including target and special resource ownership.
- SkyUI MCM with six pages.
- Offline fake-game harness and an actual-DLL API probe (2619 + 15 checks recorded in `docs/SOURCE-VALIDATION.json`).
- This repository: the sources laid out for GitHub, with the original's licence and an unmodified copy of the original
  source in `upstream/`.
