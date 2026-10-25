# Status - TrueHUD LE 1.1.10 (Test 7)

**Target:** Skyrim LE 1.9.32.0.8, SKSE 1.7.3, MSVC v143 Win32. Native port of Ersh's TrueHUD 1.1.10,
GPL-3.0-or-later with exceptions. No SE, AE, VR or CommonLibSSE-NG SDK is used.

## Changes since Test 5

- Crowd work: bounded staggered LOS refreshes, one steady sampling owner, one settings snapshot per batch,
  disabled-feature sampling skip and event-only successful loot resolution.
- Boss/UI work: fix repeated standalone shout configuration, skip unchanged transforms/depth sorts and isolate
  per-actor color configuration changes. Failed calls retry; resource updates and animations stay live.
- Logging is off by default. Optional bounded phase timings distinguish sampling, UI work, movie advance and render.
  Defaults: [`config/TrueHUD_LE.ini`](../config/TrueHUD_LE.ini); details: [`PERFORMANCE.txt`](PERFORMANCE.txt).

## Validation

Test 7 was independently rebuilt before its mod archive was delivered; provenance is in
[`TEST7-RELEASE-VALIDATION.json`](TEST7-RELEASE-VALIDATION.json). The repository retains its own build layout;
its separate local rebuild and current source hashes are recorded in [`SOURCE-VALIDATION.json`](SOURCE-VALIDATION.json).
The repository layout was also rebuilt locally: 2601 harness checks, one late-exit callback and 15 DLL API checks
all pass. GitHub Actions repeats the build, harness and actual-DLL API probe on each push. Local verification does not mean
that a GitHub run has already finished: consult the [Build workflow](../.github/workflows/build.yml) for CI status.

| Workshop Test 7 check | Result |
|---|---|
| Offline harness | 2601 checks, 0 failures |
| Late-exit callback | 1 check, 0 failures |
| Actual DLL Query/API consumer, without calling SKSE Load | 15 checks, 0 failures |
| Compiled bug-restoration mutants | 76 rejected; restored baseline passes |
| Boss/shout UI fixture, 120 frames | 1 shout configuration, 1 boss configuration, 2 transform writes, 1 depth sort, 120 live updates |
| Crowd fixture, 128 handles / 60 ticks | 1351 LOS calls versus 7680 per-frame baseline; post-first peak 32 |
| Native/AS2 source and compiled-method contracts | 51 pairs, 0 errors; 2 integration mutants rejected |
| LE engine signature spans | 35 checked against the reference executable |

## Reports and limits

The user reported Test 5 appeared functional, then reported substantial crowd and boss-display FPS drops. Test 7
fixes concrete repeated-call defects reproduced by production-code fixtures; **in-game FPS improvement is not yet
verified**. Fake GFx clips do not execute AS2 or measure tessellation, fonts, GPU work or HUD-mod compatibility.
Optional timings are inclusive wall time, can include engine/GPU waits, and are not GPU timers or direct FPS.

Complete SE feature parity is not claimed. Physical-hit emphasis and original SE per-target DOT aggregation remain
outside this build. Runtime ESP/SEQ, compiled Papyrus and converted SWF assets are in the complete mod archive,
not this source repository. AS2 bodies are unchanged; only the SWF version header was adapted to 10. Rebuilding
movies requires the external Flash authoring toolchain. No game executable, masters, extracted vanilla movies,
private logs, runtime DLLs or analysis caches are committed.
