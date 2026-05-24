# RE7 TDB49 Working Build Memory

Last updated: 2026-05-24

## What This Fork Is

This fork is a private integration branch for practical Resident Evil 7 VR quality-of-life work, focused on the DX11 non-RT / TDB49 build of RE7 with Virtual Desktop, VDXR/OpenXR, and Quest 3.

The goal is not upstream generality right now. The goal is a working, easy-to-distribute RE7 VR build with our snap turn, comfort vignette, menu, and controller behavior.

## Branch Truth

- `origin/dev` is intentionally based on the upstream v1.2 / TDB49-era REFramework line.
- `origin` is the `tsilva/REFramework-chill` fork.
- `upstream` is the official `praydog/REFramework` repository.
- Do not pull, merge, or rebase current `upstream/master` into `origin/dev` for normal RE7 releases.
- Fetch `upstream/master` and tags only for official-reference comparison and source-version metadata unless the user explicitly asks to port this work to a newer upstream line.
- Do not move `origin/dev` back to the newer universal/v2 REFramework line for this work.
- Any local `master` branch is an official-reference branch only, not the RE7 release baseline.
- The working TDB49 release commit is `49604e9f` (`Package working RE7 TDB49 build`).
- The working release tag is `re7-tdb49-openxr-v1.1.6`.
- The historical working release URL was `https://github.com/tsilva/REFramework/releases/tag/re7-tdb49-openxr-v1.1.6`; new releases should be published from `https://github.com/tsilva/REFramework-chill`.

## Critical Discovery

The RE7 install we support is the DX11 non-RT / TDB49 build. It must mimic upstream's `RE7_TDB49.zip`, not upstream's newer `RE7.zip` universal build.

The newer universal/v2 line can boot parts of REFramework, but it caused failures such as:

- REFramework overlay stuck at "currently initializing".
- Havok/provider intro hangs.
- VR not entering correctly.
- REFramework menu appearing but no VR mode.
- Crashes around D3D11/message hook attempts.
- Missing snap turn and comfort vignette when using pure upstream TDB49.

The actual functional recipe is:

- v1.2 / TDB49-era RE7 target.
- Our local commits after upstream `v1.2`.
- Old-size `openxr_loader.dll`.
- Compatible v1.2 `scripts/` copied to `reframework/autorun`.
- Current tested `re2_fw_config.txt` included at archive root.
- No `openvr_api.dll` in the OpenXR/VDXR package.

## Local Patch Stack After Upstream v1.2

These commits are the local behavior we want on top of upstream v1.2:

- `f1f19cff` Add RE7 Quest snap turning
- `32e6024f` Add stereo movement comfort vignette
- `d0c509c0` Fix OpenVR menu controller shortcut restore
- `9eb4726c` Polish RE7 OpenVR menu and comfort vignette
- `4666b6de` Lock RE7 OpenVR overlay panning
- `1a37d510` Add RE7 OpenXR menu support
- `49604e9f` Package working RE7 TDB49 build

## Working Build Command

The working local build used Visual Studio 2022, the RE7 target, and `CMKR_SKIP_GENERATION=ON`.

Important: do not let `cmkr` regenerate `CMakeLists.txt` for this TDB49 build. Regeneration produced a non-working artifact shape.

In PowerShell, the successful local configure/build was equivalent to:

```powershell
$cmake = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S . -B .\build -G "Visual Studio 17 2022" -A x64 -DDEVELOPER_MODE=ON -DCMKR_SKIP_GENERATION=ON
& $cmake --build .\build --config Release --target RE7 --parallel
```

In the Codex sandbox, MSBuild may fail if both `PATH` and `Path` exist in the process environment. Clear `PATH` before setting `Path` when launching through `cmd` if needed.

Fast path from this Codex workspace:

```powershell
Remove-Item Env:PATH -ErrorAction SilentlyContinue
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build .\build --config Release --target RE7 --parallel
```

This is the command that successfully built `build\bin\RE7\dinput8.dll` on 2026-05-23 after the plain CMake build hit the duplicate `PATH`/`Path` MSBuild error.

## Fast OpenXR Game Deploy

Use this for quick local testing after building `RE7`. The game folder used here is:

```text
D:\SteamLibrary\steamapps\common\RESIDENT EVIL 7 biohazard
```

Deploy the OpenXR layout by copying:

```text
build\bin\RE7\dinput8.dll -> game\dinput8.dll
build\bin\RE7\openxr_loader.dll -> game\openxr_loader.dll
release\re2_fw_config.txt -> game\re2_fw_config.txt
release\re7_config.ini -> game\re7_config.ini
scripts\* -> game\reframework\autorun\
```

Then remove this file from the game folder if present:

```text
openvr_api.dll
```

Recommended deployment flow:

1. Back up the current game-folder `dinput8.dll`, `openxr_loader.dll`, `openvr_api.dll`, `re2_fw_config.txt`, `re7_config.ini`, `reframework_revision.txt`, and `reframework\autorun`.
2. Copy the fresh build outputs/config/scripts listed above.
3. Delete `openvr_api.dll` from the game folder so VDXR/OpenXR is selected.
4. Write `reframework_revision.txt` with the local branch, commit, target, and deploy timestamp.

The 2026-05-23 deploy backup naming pattern was:

```text
reframework-openxr-backup-YYYYMMDD-HHMMSS
```

## Quest 3 OpenXR LT / Weapon Dial Fix

Confirmed working on 2026-05-23 with Quest 3 + Virtual Desktop / VDXR OpenXR.

Symptom:

- RT worked normally.
- LT + left stick selected weapons.
- Plain LT did not work in contexts that need the left trigger by itself, such as the Jack garage car sequence where LT accelerates and RT reverses.

What was actually happening:

- On this OpenXR setup, left trigger weapon select was coming through `WeaponDial_Start`, not reliably through the generic `Trigger` action.
- Removing the OpenXR `weapondial_start` binding restored neither the desired behavior nor weapon selection; it broke LT + left-stick weapon selection.
- C++ RE2/RE3 input injection was not enough for RE7, because RE7's effective pad mapping is driven through `scripts/re8_vr.lua`.

Working fix:

- Keep the OpenXR binding in `src/mods/vr/runtimes/OpenXR.hpp`:

```cpp
{"/user/hand/left/input/trigger", "weapondial_start"},
```

- In `src/mods/VR.cpp`, for OpenXR, treat `m_action_weapon_dial` on the left hand as a fallback source for `is_left_trigger_down`.
- In `scripts/re8_vr.lua`, define `is_left_trigger_active` as:

```lua
local is_left_trigger_active = vrmod:is_action_active(action_trigger, left_joystick) or vrmod:is_action_active(action_weapon_dial, left_joystick)
```

- Also make plain LT emit normal pad trigger state in Lua:

```lua
device:call("set_AnalogL", 1.0)
cur_button = cur_button | via.hid.GamePadButton.LTrigBottom
```

Result:

- LT + left stick weapon selection still works.
- Plain LT works again in game contexts that require left trigger alone.

## Known Good Artifact Shape

Known working sizes from the verified build:

- `dinput8.dll`: `5,414,912` bytes
- `openxr_loader.dll`: `360,960` bytes

Known bad clean build sizes seen during debugging:

- `dinput8.dll`: `5,794,816` bytes
- `openxr_loader.dll`: `647,168` bytes

If the artifact sizes drift to the bad shape, suspect regenerated CMake/build configuration.

## Package Layout

The release asset must be named:

```text
RE7_TDB49.zip
```

The zip must contain these root entries:

```text
dinput8.dll
openxr_loader.dll
re2_fw_config.txt
reframework_revision.txt
reframework/autorun/re8_vr.lua
reframework/autorun/re2_sharpness_removal.lua
reframework/autorun/re2_vr_grenade.lua
reframework/autorun/re2_vr_melee.lua
reframework/autorun/utility/GameObject.lua
reframework/autorun/utility/ManagedObjectDict.lua
reframework/autorun/utility/RE2.lua
reframework/autorun/utility/RE7.lua
reframework/autorun/utility/RE8.lua
reframework/autorun/utility/Statics.lua
reframework/autorun/vr/VRControllerManager.lua
```

Do not include `openvr_api.dll` in the OpenXR/VDXR package. Including or leaving `openvr_api.dll` changes runtime selection and was not the final working path.

Use:

```powershell
powershell -ExecutionPolicy Bypass -File .\dev\package-re7-tdb49.ps1 -BuildDir .\build -Configuration Release
```

The package script includes `release/re2_fw_config.txt` automatically.

The package script also includes the RE7 game config from:

```text
release/re7_config.ini
```

and distributes it at the archive root as:

```text
re7_config.ini
```

## Why Autorun Scripts Matter

Do not leave `reframework/autorun` empty for the release.

Observation from testing:

- With empty `autorun`, OpenXR controller axes were detected.
- Snap turn worked.
- Movement comfort vignette appeared when moving the left stick.
- But RE7 movement/buttons did not work and player hands did not appear.

Reason:

- Snap turn and vignette are handled inside REFramework C++ from OpenXR axes.
- RE7 motion control glue, hand behavior, and pad input injection rely on compatible Lua autorun scripts, especially `re8_vr.lua`.

Use the v1.2-compatible `scripts/re8_vr.lua` from this repo. Do not use the newer upstream TDB49 `re8_vr.lua` as-is with this DLL.

## Upstream Script Trap

The upstream extracted `RE7_TDB49.zip` scripts caused this popup with our old DLL:

```text
sol: runtime error: ...\reframework\autorun\re8_vr.lua:9: field 'get_tdb_version' is not callable (a nil value)
```

That happened because the upstream script calls:

```lua
sdk.get_tdb_version()
```

The compatible v1.2 script in this repo does not call `sdk.get_tdb_version()` and was part of the final working setup.

## Deployed Test Path

The test install path used during debugging was:

```text
D:\SteamLibrary\steamapps\common\RESIDENT EVIL 7 biohazard
```

The final working deploy contained:

```text
DINPUT8.dll / dinput8.dll
openxr_loader.dll
re2_fw_config.txt
reframework/autorun/*
```

and did not contain:

```text
openvr_api.dll
```

## Config

The working user config is tracked at:

```text
release/re2_fw_config.txt
```

It should be distributed at the archive root as:

```text
re2_fw_config.txt
```

This config enables the tested defaults, including:

- `VR_SnapTurn=true`
- `VR_SnapTurnAngle=45.000000`
- `VR_SnapTurnThreshold=0.500000`
- `VR_ComfortVignette=true`
- `VR_2DUIDistance=1.500000`
- `VR_2DUIScale=12.000000`
- `VR_WorldSpaceUIScale=15.000000`
- `RE8VR_HideArms=false`
- `RE8VR_HideLowerBody=true`
- `RE8VR_HideUpperBody=true`

## RE7 Game Config File

The bundled RE7 game config is:

```text
release/re7_config.ini
```

This is Capcom/RE Engine's root-level `re7_config.ini`, not REFramework's `re2_fw_config.txt`. It is a plain INI file with bracketed sections and `key=value` entries. Edit it with INI semantics: preserve sections, key spelling, value casing, booleans as `true`/`false` where already used, numeric values as bare integers/floats, and vectors in the game's parenthesized float form such as `(0.000000,0.000000)`.

Known sections in our bundled file:

- `[Render]`: API/platform selection and crash state. Important keys include `Capability=DirectX12` and `TargetPlatform=DirectX11`; for the TDB49/DX11 RE7 package, keep `TargetPlatform=DirectX11`.
- `[RenderConfig]`: the main graphics/options payload. Includes window/fullscreen mode, selected display mode index, FOV, VSync/framerate, AA, motion blur/DOF, shadow/mesh/texture quality, AO/SSR/SSSS, lens/chromatic/film settings, color space, RT/FSR-era options, and `PlayerFOV`.
- `[Control_<number>]`: mouse/control settings. The suffix appears to be a profile/user-specific numeric id, so preserve it unless deliberately replacing the whole generated control profile.
- `[KeyBind_<number>]`: keyboard bindings using `KBxx=<integer>` entries. The suffix should match the control profile id when present.
- `[Render/Adapter]`: generated GPU identity for the machine that produced the config. This is hardware-specific and should not be treated as game logic.
- `[Render/Display]`: generated display mode enumeration. `FullScreenDisplayMode` in `[RenderConfig]` indexes into this list. Because this is monitor-specific, prefer safe windowed defaults over hard-coding a display mode that may not exist for another player.
- `[Display]`: display/HDR flags such as `HDRMode`.

Online references confirm normal RE7 installs place `re7_config.ini` beside the game executable under the Steam game folder, and users commonly edit `[RenderConfig]` keys such as `FullScreenMode`, `WindowMode`, and `FullScreenDisplayMode`, plus `[Display]`/`HDRMode`, to recover from launch/display issues.

## What Worked In Final Test

After deploying the size-correct DLL/loader and restoring compatible v1.2 autorun scripts:

- Game entered VR.
- Player hands appeared.
- Controllers worked.
- Snap turn worked.
- Comfort vignette worked.
- Game movement/buttons worked.

## RE7 Inventory Examine Snap-Turn Conflict

Confirmed issue: when examining/rotating an item from the RE7 inventory, the right stick rotates the item but also triggered OpenXR snap turn. This was not a shadow-puzzle-specific event task and was not reliably detected by `num_active_tasks` or cutscene/controller state.

Cause: inventory item examine is regular inventory UI pad input. `scripts/re8_vr.lua` still passes `vr_right_stick_axis` to the game with `set_RawAxisR` / `set_AxisR`, while C++ snap turn also watches the same right-stick horizontal axis globally.

Working fix: expose a timed C++ suppression API (`VR::suppress_snap_turn_for`) and call it from `scripts/re8_vr.lua` whenever RE7 inventory is open. The deployed working value was `vrmod:suppress_snap_turn_for(0.25)`. This preserves right-stick item rotation while blocking snap turn during inventory examine. Keep `openvr_api.dll` absent for OpenXR deploys.

## Current Comfort Issue Under Investigation

Target runtime remains PCVR through Virtual Desktop / VDXR OpenXR on Quest 3.

Reported symptom: during RE7 death/fall moments, the last visible frame can feel like forced eye crossing. A similar effect can appear on certain subtitles. Treat this as excessive stereo disparity from content being rendered too close to the HMD, not a flat-monitor artifact.

Likely causes in this fork:

- RE7 can drive the camera very close to body/floor geometry during death or scripted camera transitions.
- Some GUI/subtitle-like elements are converted from screen UI to VR world UI, and RE7 `UIWorldPosAttach` elements previously used `_NowTargetPos` directly without a minimum HMD distance.

Comfort patch direction:

- Keep RE7 VR projection near-Z at or above `0.1f` so extremely close death-frame geometry clips instead of stereo-fusing at face distance.
- Default/package `VR_2DUIDistance` to `1.5`.
- Clamp RE7 world-attached GUI to at least about one meter from the HMD while preserving direction.
- If the "You are dead" menu is visible, suppress the RE7 scene and post-effect draw layers behind it for a rolling short comfort window so the menu remains visible without the crossed-eye death-frame backdrop.
- Follow-up observation: the remaining bad image appears when the game-over UI swaps the live scene to a captured/frozen last-frame image. Suppress likely RE7 game-over capture/backdrop GUI elements too, especially names containing backdrop/capture/screenshot/image-plane/blur clues and `GameOverSceneTimelineBehavior` / `BlurFilter` components.

Cutscene comfort vignette confirmed working in-game:

- Add saved C++ setting `VR_ComfortVignetteCutscenes=true`, exposed in the VR menu as `Auto Vignette in Cutscenes`.
- Keep `VR_ComfortVignette` as the master kill switch; the cutscene toggle only contributes a target when the main vignette is enabled.
- Expose `vrmod:set_cutscene_vignette_active(bool)` from `VR::set_cutscene_vignette_active`.
- Let Lua report the already-known cutscene state instead of re-detecting it in C++:
  - `scripts/utility/RE8.lua`: `set_vr_cutscene_state(re8vr.is_in_cutscene)`.
  - `scripts/utility/RE7.lua`: `set_vr_cutscene_vignette(re7.is_in_cutscene)`.
- In `VR::update_comfort_vignette`, include `cutscene_target` in the existing `std::max({ movement_target, turn_target, snap_turn_target, cutscene_target })`.
- Targeted RE7 Release build succeeded after this wiring, and user confirmed the behavior worked in-game.

## GitHub Release

Final functional release created:

```text
Historical release URL:

https://github.com/tsilva/REFramework/releases/tag/re7-tdb49-openxr-v1.1.6

New fork URL:

https://github.com/tsilva/REFramework-chill
```

Release asset:

```text
RE7_TDB49.zip
```

Release notes should mention this is targeted at RE7 DX11 non-RT / TDB49, Virtual Desktop, VDXR/OpenXR, and Quest 3.
