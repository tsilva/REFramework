# RE7-antipuke Setup Guide

RE7-antipuke is an opinionated Resident Evil 7 VR build based on Praydog's excellent REFramework VR work.

The goal is simple: make RE7 more playable in VR for people who find the stock VR mod experience too intense. RE7 is an aggressive VR game because of fast camera cuts, unstable scripted camera transitions, forced motion, and other comfort-hostile moments. This build changes the experience toward comfort, even when that means making choices that may not be ideal for every player or every RE Engine game.

This build is currently meant for RE7 first. It includes OpenVR and OpenXR support, but it is tuned for OpenXR through Virtual Desktop with VDXR selected, using a Quest 3. Compatibility with other Resident Evil games may be broken or untested. The plan is to go through the other games later.

## What This Build Changes

- Adds a comfort vignette for movement, rotation, snap turning, and cutscenes.
- Adds snap turning on the joystick, with discrete 45 degree turns by default.
- Ships a REFramework config with the comfort settings I currently consider best for RE7.
- Ships a RE7 config with the game settings I currently consider best for a similar PC/Quest 3 setup.
- Keeps both `openxr_loader.dll` and `openvr_api.dll` in the package, even though the recommended path is OpenXR + VDXR.

## Recommended Hardware And Runtime

This is the setup I use. I play in front of the router to maximize signal quality. The PC connects to the router with Ethernet, and the Quest 3 connects to the router over Wi-Fi 6/6E. Streaming is done with Virtual Desktop using the VDXR runtime. With this setup I get stable 30-40 ms latency.

PC and network:

- RTX 4090
- 64 GB DDR5
- PC connected to router through Ethernet
- Wi-Fi 6E router
- Quest 3
- Player near the router, or wherever the headset gets the strongest signal

Virtual Desktop:

- Quality: Godlike
- Codec: AV1 10-bit
- Fallback codec: HEVC 10-bit
- Bitrate: 150-200+ Mbps
- Sharpening: 0%
- Refresh rate: 90 Hz
- SSW: Off if stable
- Runtime: VDXR

## Install

1. Install Resident Evil 7 from Steam.
2. Switch the game to the DX11 / non-RT build. This is critical: RE7-antipuke is built and tuned for the older DirectX 11 non-ray-tracing version, not the newer ray-tracing / enhanced update. In Steam, open `Library`, right-click `Resident Evil 7 Biohazard`, choose `Properties...`, open `Betas`, and select `dx11_non-rt` from the beta dropdown. No beta password is required. Close the Properties window and wait for Steam to finish updating the game before installing the mod.
3. If you already had RE7 installed, expect Steam to redownload or replace some game files when you switch branches. Capcom's Steam announcement also notes that some in-game option settings can reset during this rollback.
4. Download the RE7-antipuke build zip. Package names are prefixed with the upstream REFramework source-release version this fork is based on, for example `v1.2-RE7-antipuke-v1.1.6.zip`. Older packages may still be named `RE7_TDB49.zip`.
5. Open the RE7 game folder, the folder that contains the game executable.
6. Back up any existing REFramework files in that folder, especially:
   - `dinput8.dll`
   - `openxr_loader.dll`
   - `openvr_api.dll`
   - `re2_fw_config.txt`
   - `re7_config.ini`
   - `reframework`
7. Extract the full zip into the RE7 game folder.
8. Confirm these files are now beside the game executable:
   - `dinput8.dll`
   - `openxr_loader.dll`
   - `openvr_api.dll`
   - `re2_fw_config.txt`
   - `re7_config.ini`
   - `reframework\autorun`
9. Start Virtual Desktop on the Quest 3 and connect to the PC.
10. In Virtual Desktop, use VDXR as the OpenXR runtime.
11. Launch RE7.

Reference: Capcom's Steam announcement for the DirectX 11 non-ray-tracing rollback lists the same `dx11_non-rt` beta branch and update flow: <https://store.steampowered.com/news/posts/?enddate=1655351742&feed=steam_community_announcements>.

## Recommended RE7 Settings

The package includes `re7_config.ini` so you can start from the settings I use. The important shipped values are:

- Game branch: Steam `dx11_non-rt`
- Render target: DirectX 11 (`TargetPlatform=DirectX11`; the captured hardware capability may still say DirectX 12)
- Window mode: Normal window, not fullscreen
- Resolution scale / image quality: 100% (`ImageQualityRate=1`)
- Framerate: Variable
- VSync: Off
- Anti-aliasing: SMAA
- Motion Blur: Off
- Depth of Field: Off
- Chromatic Aberration: Off
- Lens Flare: Off
- Ambient Occlusion: Off
- Screen Space Reflections: Off
- Subsurface Scattering: Off
- Shadows: Highest
- Dynamic Shadows: On
- Shadow Cache: On
- Mesh Quality: Highest
- Texture Quality: Highest
- Texture Filtering: `Anisotropic2` / secondary `Anisotropic4`
- Transparent Buffer Quality: None
- God Rays: On
- Bloom: On
- Lens Distortion: On
- Film Grain: On
- Upscaling: None

## Optional Texture Mod

For better texture quality, I recommend installing [4K HD Upscaled Textures](https://www.nexusmods.com/residentevil7/mods/75) from Nexus Mods. The Nexus page describes it as 4K AI-upscaled textures for DX11, and notes that this version is for the pre-ray-tracing version of the game, which is the same `dx11_non-rt` branch required by RE7-antipuke.

Install the texture mod only after Steam has finished switching RE7 to `dx11_non-rt`, and follow the install instructions on the Nexus page for that mod.

## Recommended REFramework Settings

The package includes `re2_fw_config.txt` with my current comfort defaults:

- Target runtime path: OpenXR through Virtual Desktop / VDXR
- OpenXR resolution scale: 1.0
- Movement/Turn Vignette: On
- Auto Vignette in Cutscenes: On
- Vignette Strength: 1.0
- Vignette Range: 80.0
- Vignette Begin Angle: 24.0
- Vignette End Angle: 54.0
- Vignette Fade In: 0.08
- Vignette Fade Out: 0.25
- Snap Turning: On
- Snap Turn Angle: 45 degrees
- Snap Turn Threshold: 0.5
- Joystick Deadzone: 0.15
- Decoupled Camera Pitch: On
- Head Oriented Audio: On
- Hide Upper Body: True
- Hide Lower Body: True
- Hide Arms: False
- Auto Hide Upper Body in Cutscenes: True
- Auto Hide Lower Body in Cutscenes: True
- 2D UI Distance: 1.5
- 2D UI Scale: 12.0
- World-Space UI Scale: 15.0
- Force Uncap FPS: On
- Force Disable TAA: On
- Force Disable Motion Blur: On
- Force Disable V-Sync: On
- Force Disable Lens Distortion: On
- Force Disable Volumetrics: On
- Force Disable Lens Flares: On
- Force Enable Dynamic Shadows: On
- Allow Engine Overlays: On
- Desktop Recording Fix: On

The intent is to make fast movement, rotation, and cutscenes less punishing. These settings are subjective; this is not a neutral or universal REFramework build.

## Notes

- This build is for comfort first. It may trade visual purity or cross-game compatibility for a calmer RE7 VR experience.
- OpenVR is included, but OpenXR + Virtual Desktop + VDXR is the target path.
- The bundled config files are starting points. If your hardware or tolerance differs from mine, adjust the game graphics settings, Virtual Desktop bitrate, and vignette strength.
- Keep a backup of your original RE7 config if you already have a setup you care about.
