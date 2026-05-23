# RE7 TDB49 Working Build Memory

Last updated: 2026-05-23

## What This Fork Is

This fork is a private integration branch for practical Resident Evil 7 VR quality-of-life work, focused on the DX11 non-RT / TDB49 build of RE7 with Virtual Desktop, VDXR/OpenXR, and Quest 3.

The goal is not upstream generality right now. The goal is a working, easy-to-distribute RE7 VR build with our snap turn, comfort vignette, menu, and controller behavior.

## Branch Truth

- `tsilva/dev` is intentionally based on the upstream v1.2 / TDB49-era REFramework line.
- Do not move `tsilva/dev` back to the newer universal/v2 REFramework line for this work.
- `master` should stay aligned with upstream `origin/master`.
- The working TDB49 release commit is `49604e9f` (`Package working RE7 TDB49 build`).
- The working release tag is `re7-tdb49-openxr-v1.1.6`.
- The working release URL is `https://github.com/tsilva/REFramework/releases/tag/re7-tdb49-openxr-v1.1.6`.

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

## What Worked In Final Test

After deploying the size-correct DLL/loader and restoring compatible v1.2 autorun scripts:

- Game entered VR.
- Player hands appeared.
- Controllers worked.
- Snap turn worked.
- Comfort vignette worked.
- Game movement/buttons worked.

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

## GitHub Release

Final functional release created:

```text
https://github.com/tsilva/REFramework/releases/tag/re7-tdb49-openxr-v1.1.6
```

Release asset:

```text
RE7_TDB49.zip
```

Release notes should mention this is targeted at RE7 DX11 non-RT / TDB49, Virtual Desktop, VDXR/OpenXR, and Quest 3.
