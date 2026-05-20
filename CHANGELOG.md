# Changelog

This file tracks local REFramework VR work before it is merged upstream.

## 2026-05-20

### RE7 Quest 3 / OpenVR comfort and menu work

Branch: `codex/re7-v1.2-quest3-snapturn`

- Added RE7 snap turning with configurable enable toggle, snap angle, and stick threshold.
- Added a D3D11 stereo comfort vignette driven by movement.
- Extended the vignette to smooth turning and snap turning.
- Added a simplified `Vignette Range` setting from `0%` to `100%`.
- Set the RE7 default vignette range to `80%`.
- Widened the REFramework in-game menu.
- Disabled horizontal scrollbars in Lua-created ImGui windows and child windows.
- Added an OpenVR both-trigger shortcut to open the REFramework menu.
- Restored the original OpenVR hand-point menu activation path.
- Disabled OpenVR smooth-scroll forwarding into the overlay to prevent controller drag/pan from damaging the menu layout.
- Made the REFramework menu non-movable and non-collapsible while shown in VR.

Known status:

- Opening the menu works by hand pointing and double trigger in OpenVR.
- Closing the menu with double trigger is still unreliable.
- The menu layout damage from dragging/panning the overlay is fixed.

### RE7 OpenVR startup dashboard experiment

Branch/worktree: `.worktrees/reframework-v1.2-snapturn`

Status: experimental, not committed.

- Started the OpenVR overlay internally closed and hidden until the runtime is ready.
- Kept the overlay out of SteamVR dashboard tabs and dashboard visibility.
- Ignored startup `DashboardActivated` pause events during an initial grace period.
- Suppressed startup system-button pause mapping during the same grace period.
- Temporarily disabled SteamVR `startDashboardFromAppLaunch` around OpenVR scene init, then restored the original setting.

Known status:

- The SteamVR dashboard still appeared during VR startup in testing, so this is not yet a confirmed fix.

### RE8 cutscene comfort and turning

Branch: `codex/re8-cutscene-smooth-turn`

- Added `Smooth Cutscene Vertical Camera`, enabled by default.
- Dampened only the RE8 cutscene camera joint Y position.
- Left cutscene rotation and larger scene motion intact.
- Added `Smooth Turn Speed`, default `0.5`.
- Scaled only the game's smooth-turn/right-stick X axis.
- Kept snap turning on the raw stick value with the same angle and threshold behavior.

### OpenVR hand menu targeting

Branch: `codex/re8-cutscene-smooth-turn`

- Expanded the closed-menu hand/head targeting area from the center `25%..75%` window to `15%..85%`.
- Allowed closed-menu intersection checks even while another controller action is down.
- Accumulated controller intersections instead of allowing later controller checks to overwrite earlier hits.

### OpenXR support pass

Branch: `codex/re8-cutscene-smooth-turn`

- Added an OpenXR both-trigger shortcut to toggle the REFramework menu.
- Kept snap turn and smooth turn on the shared OpenVR/OpenXR input path.
- Added the movement/turn/snap-turn comfort vignette settings to the shared VR config.
- Applied the comfort vignette in the D3D11 eye texture path so OpenXR receives the same comfort mask without touching engine GUI/tone-map state.
- Widened the REFramework menu and disabled mouse-wheel panning on the main menu window.
- Disabled horizontal scrollbar flags for Lua-created ImGui windows and child windows.
