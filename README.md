# REFramework-antipuke

Opinionated RE7 VR comfort fork of Praydog's REFramework. Experimental, Quest 3 / Virtual Desktop / VDXR focused.

This is a fork of [praydog/REFramework](https://github.com/praydog/REFramework). REFramework-antipuke would not exist without Praydog's original REFramework VR work.

REFramework-antipuke is for playing Resident Evil 7 in VR with more comfort. It adds comfort-focused defaults and behavior for a game that can be rough in VR because of fast camera cuts, unstable scripted camera transitions, forced motion, and intense cutscenes.

This build is currently tested only on RE7. It may break other RE Engine games.

## Quick Start

1. Install RE7 on Steam.
2. Switch RE7 to the Steam `dx11_non-rt` beta branch. This is required.
3. Download the latest REFramework-antipuke build. Current artifacts may still be named `RE7-antipuke`; if GitHub gives you an artifact wrapper zip, unzip it first and use the versioned mod zip inside.
4. Extract the versioned mod zip into the RE7 install folder, next to `re7.exe`.
5. For the recommended OpenXR + Virtual Desktop + VDXR path, remove or rename `openvr_api.dll` after extraction. Keep `openxr_loader.dll`.
6. Start Virtual Desktop, select VDXR as the OpenXR runtime, connect the Quest 3, and launch RE7.

Read the install notes below before replacing existing REFramework files.

## What This Changes

- Movement, rotation, snap-turn, and cutscene comfort vignette.
- Joystick snap turning, 45 degrees by default.
- RE7-focused REFramework comfort config.
- RE7 game config tuned for the setup below.
- OpenXR and OpenVR files in the package, with OpenXR + VDXR recommended.

## Recommended Setup

This is the setup I use:

- Quest 3
- Virtual Desktop with VDXR
- PC connected to router over Ethernet
- Quest 3 connected over Wi-Fi 6/6E
- Player near the router or wherever signal is strongest
- RTX 4090, 64 GB DDR5

Virtual Desktop settings:

- Quality: Godlike
- Codec: AV1 10-bit, or HEVC 10-bit fallback
- Bitrate: 150-200+ Mbps
- Sharpening: 0%
- Refresh rate: 90 Hz
- SSW: Off if stable

With this setup I get stable 30-40 ms latency.

## Install

### 1. Use The Required RE7 Version

REFramework-antipuke targets the older DX11 / non-ray-tracing build of RE7. The newer ray-tracing / enhanced update is not the target.

In Steam:

1. Open `Library`.
2. Right-click `Resident Evil 7 Biohazard`.
3. Choose `Properties...`.
4. Open `Betas`.
5. Select `dx11_non-rt`.
6. Close Properties and wait for Steam to finish updating the game.

No beta password is required. If RE7 was already installed, Steam may redownload files and reset some game options.

Reference: Capcom's Steam announcement for the DX11 rollback lists the same branch and update flow: <https://store.steampowered.com/news/posts/?enddate=1655351742&feed=steam_community_announcements>.

### 2. Install The Mod Files

1. Download the build zip. Current RE7-targeted package names look like `v1.2-RE7-antipuke-v1.1.6.zip`; older packages may be named `RE7_TDB49.zip`.
2. If you downloaded a GitHub Actions artifact named `RE7-antipuke`, unzip that artifact first. Install the versioned mod zip inside it.
3. Open the RE7 folder that contains `re7.exe`. A default Steam path is usually `C:\Program Files (x86)\Steam\steamapps\common\RESIDENT EVIL 7 biohazard`.
4. Back up any existing REFramework files there:
   - `dinput8.dll`
   - `openxr_loader.dll`
   - `openvr_api.dll`
   - `re2_fw_config.txt`
   - `re7_config.ini`
   - `reframework`
5. Extract the full versioned mod zip into the RE7 folder.
6. Confirm these files are beside `re7.exe`:
   - `dinput8.dll`
   - `openxr_loader.dll`
   - `openvr_api.dll`
   - `re2_fw_config.txt`
   - `re7_config.ini`
   - `reframework\autorun`
7. For OpenXR + Virtual Desktop + VDXR, remove or rename `openvr_api.dll`. REFramework tries OpenVR first when that DLL is present.
8. Keep `openxr_loader.dll`.

Do not rename `re2_fw_config.txt`. The name is inherited from REFramework's config layout, but this package uses it for the shipped REFramework-antipuke settings.

### 3. Launch

1. Start Virtual Desktop Streamer on the PC.
2. Set Virtual Desktop's OpenXR runtime to VDXR.
3. Connect the Quest 3 through Virtual Desktop.
4. Launch RE7.

## Shipped Defaults

The package includes `re7_config.ini` and `re2_fw_config.txt` as starting points.

RE7 defaults:

- Steam branch: `dx11_non-rt`
- Render target: DirectX 11
- Resolution scale: 100%
- Window mode: normal window
- VSync, motion blur, depth of field, chromatic aberration, lens flare, ambient occlusion, SSR, subsurface scattering, and upscaling: off
- Anti-aliasing: SMAA
- Shadows, mesh quality, texture quality: highest
- Dynamic shadows, shadow cache, god rays, bloom, lens distortion, film grain: on

REFramework defaults:

- Target runtime path: OpenXR through Virtual Desktop / VDXR
- Movement/turn vignette and cutscene vignette: on
- Vignette strength: 1.0
- Vignette range: 80.0
- Vignette begin/end angle: 24.0 / 54.0
- Snap turning: on
- Snap turn angle: 45 degrees
- Snap turn threshold: 0.5
- Joystick deadzone: 0.15
- Decoupled camera pitch and head-oriented audio: on
- Hide upper body and lower body: true
- Hide arms: false
- 2D UI distance/scale: 1.5 / 12.0
- World-space UI scale: 15.0

These defaults are subjective and comfort-first.

## Optional Texture Mod

For better texture quality, I recommend [4K HD Upscaled Textures](https://www.nexusmods.com/residentevil7/mods/75) from Nexus Mods. Install it only after Steam has switched RE7 to `dx11_non-rt`.

## Troubleshooting

- Mod does not load or crashes: confirm Steam is using `dx11_non-rt`.
- OpenVR loads instead of OpenXR: remove or rename `openvr_api.dll` and keep `openxr_loader.dll`.
- Game settings reset: switch to `dx11_non-rt`, then extract the mod zip again so `re7_config.ini` is restored.
- Existing REFramework install conflicts: move old REFramework files out of the RE7 folder, then extract this package again.
- Wireless latency or stutter: lower Virtual Desktop bitrate or quality before changing REFramework settings.

## Why This Is A Fork

These changes are distributed as a fork instead of a pull request to the original repo because they are experimental, mostly vibe coded, and currently tested only through my own in-progress RE7 playthrough. I fix issues as I find them.

This build also includes personal comfort choices that may not be desirable for everyone or appropriate for upstream REFramework.
