# Release Delta From Official Master

Generated on 2026-05-23 from branch `dev`.

This file tracks how this release branch differs from the official REFramework `master` build.

Keep it as a release manifest: before publishing a build, update this document so we can quickly answer what is custom in our build, why it exists, and how far it diverges from the official project.

This inventory is based on the codebase diff from `master...HEAD`, not on commit history, plus the current uncommitted worktree edits present when the inventory was finalized.

## How To Use This File

- Use it when preparing release notes, GitHub release descriptions, or support/debug summaries.
- Treat each numbered entry as a user-facing or build-facing difference from official REFramework.
- Update the comparison point, diff size, and change list before every release.
- Prefer describing behavior and purpose over commit names.
- If a change is temporary, risky, hardware-specific, or release-only, call that out explicitly.

## Release Update Checklist

Before publishing a release:

1. Refresh `master` so the official baseline is current.
2. Confirm the release branch and commit that will be packaged.
3. Run `git diff --stat master...HEAD` and update the size summary.
4. Run `git diff --name-status master...HEAD` and confirm the file inventory still matches.
5. Review `git diff master...HEAD` by feature area and update the numbered change list.
6. Check `git status --short`; include any intentional uncommitted release edits or commit them before publishing.
7. Note any hardware-specific configs, temporary workarounds, or known differences from official behavior.

## Official Comparison

Comparison point:

- Current branch: `dev`
- Current HEAD: `ec37a392`
- Official baseline branch: local `master`
- Official baseline commit: `c4b13148`
- Merge base with `master`: `aebc1e34`
- Committed diff command used as the source of truth: `git diff master...HEAD`
- Additional current-worktree diff checked with: `git diff HEAD`

Before this inventory file was added, the branch differed from `master` by:

- 22 files changed
- 2000 insertions
- 67 deletions
- 4 newly added files: `MEMORY.md`, `dev/package-re7-tdb49.ps1`, `release/re2_fw_config.txt`, `release/re7_config.ini`

At finalization time, the worktree also had uncommitted edits in:

- `CMakeLists.txt`
- `MEMORY.md`
- `release/re2_fw_config.txt`
- `scripts/utility/RE7.lua`
- `scripts/utility/RE8.lua`
- `src/mods/VR.cpp`
- `src/mods/VR.hpp`

Those uncommitted edits are included below because they are part of the current codebase state.

## Release Summary

Compared with official REFramework `master`, this build is a practical Resident Evil 7 TDB49 / v1.2-era VR release branch. It adds comfort and controller behavior for Quest 3 / Virtual Desktop / VDXR / OpenXR use, keeps OpenVR menu support usable, and packages a working `RE7_TDB49.zip` artifact with known-good config files.

At a high level, this release differs from official by adding:

- RE7 TDB49 release packaging and known-good config payloads.
- Quest/VR snap turning and safeguards so it does not conflict with inventory or cutscenes.
- Movement, turn, snap-turn, and cutscene comfort vignette behavior.
- D3D11 stereo vignette rendering for reliable VR comfort output.
- RE7-specific UI distance, scale, near-clip, and game-over comfort fixes.
- OpenVR/OpenXR menu and controller compatibility fixes.
- Documentation of the working RE7 TDB49 build/deploy recipe.

## Change List

### 1. RE7 TDB49 Release Packaging

Purpose: produce a ready-to-install RE7 TDB49 zip instead of a generic multi-game dev artifact.

Files changed:

- `.github/workflows/dev-release.yml`
- `dev/package-re7-tdb49.ps1`
- `.gitignore`
- `release/re2_fw_config.txt`
- `release/re7_config.ini`

What changed:

- The GitHub Actions workflow was renamed from a generic dev release to `RE7 TDB49 Dev Release`.
- The workflow no longer builds a matrix of RE2, RE3, RE7, RE8, DMC5, and MHRISE. It builds only the `RE7` target.
- The workflow now configures with `-DCMKR_SKIP_GENERATION=ON`, builds `RE7`, runs `dev/package-re7-tdb49.ps1`, and uploads `build/release/RE7_TDB49.zip`.
- The old generic artifact compression/release steps were removed.
- A new PowerShell packaging script stages:
  - `dinput8.dll`
  - `openxr_loader.dll`
  - `openvr_api.dll`
  - `release/re2_fw_config.txt`
  - `release/re7_config.ini`
  - `scripts/` copied into `reframework/autorun`
  - `reframework_revision.txt` with branch, commit, source, target, and package metadata
- `.gitignore` now ignores `build_v1_2*/` local build directories.

Why:

- The supported RE7 install is the DX11 non-RT / TDB49 build, which needs the older v1.2-style artifact layout.
- The packaged output needs to be usable directly by the RE7 target audience, rather than matching upstream's current universal release shape.

### 2. Runtime Loader DLL Copying In Builds

Purpose: make local build output contain the VR runtime loader DLLs beside the game DLLs.

Files changed:

- `CMakeLists.txt`
- `cmake.toml`

What changed:

- Each game target now has a post-build step that copies:
  - `dependencies/openvr/bin/win64/openvr_api.dll`
  - `$<TARGET_FILE:openxr_loader>` as `openxr_loader.dll`
- `cmake.toml` was updated with the template hook that generates the same post-build copy command for cmkr-managed game targets.
- Current worktree edits changed the generated `CMakeLists.txt` post-build target/output references from hard-coded target names (`RE2`, `RE3`, `RE7`, `RE8`, `DMC5`, `MHRISE`) to `${CMKR_TARGET}`.

Why:

- The RE7 TDB49 package and local test layout need runtime loader DLLs available next to `dinput8.dll`.
- The packaging script can pick up loader DLLs from the RE7 build output instead of relying only on dependency fallback paths.
- The `${CMKR_TARGET}` form appears intended to keep generated CMake output aligned with the cmkr template rather than maintaining six separate literal target names by hand.

### 3. Release Config Payload

Purpose: include known-good REFramework and RE7 game settings in the package.

Files changed:

- `release/re2_fw_config.txt`
- `release/re7_config.ini`

What changed:

- `release/re2_fw_config.txt` was added with VR defaults for this branch, including:
  - snap turning enabled
  - 45 degree snap-turn angle
  - comfort vignette enabled
  - vignette strength, range, fade, and angle defaults
  - RE7-friendly 2D UI distance and UI scale
  - common VR rendering force flags
- `release/re7_config.ini` was added with a full tested RE7 game config.

Why:

- The release needs to land in a usable state without asking users to rediscover all VR comfort and display settings.
- The branch is targeting a specific working RE7 TDB49 setup, so the package carries the tested configuration alongside the DLLs and scripts.

### 4. Snap Turning Core

Purpose: add right-stick snap turning for RE7 VR locomotion.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`

What changed:

- Added settings:
  - `VR_SnapTurn`
  - `VR_SnapTurnAngle`
  - `VR_SnapTurnThreshold`
- Added snap-turn state:
  - `m_was_snap_turn_active`
  - `m_snap_turn_suppressed`
  - `m_snap_turn_suppressed_until`
  - `m_snap_turn_vignette_until`
- Added snap-turn functions:
  - `update_snap_turn`
  - `is_snap_turn_axis_active`
  - `is_snap_turn_suppressed`
  - `set_snap_turn_suppressed`
  - `suppress_snap_turn_for`
- `update_action_states` now calls `update_snap_turn` every frame.
- Snap turn rotates the VR rotation offset by the configured angle and adjusts standing origin so the HMD position remains stable through the turn.
- Several controller input paths now clear right-stick axis input when that axis is being consumed for snap turn.
- Snap-turn controls were added to the VR settings UI.
- Config load now repairs missing or invalid snap-turn defaults.

Why:

- Smooth right-stick turning is uncomfortable for many users in VR.
- Clearing the right-stick axis after it triggers snap turn prevents the game from also receiving the same turn input and producing double movement.

### 5. Snap-Turn Suppression For Inventory And Cutscenes

Purpose: stop snap turn from fighting RE7 inventory item examine and cutscene states.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`
- `scripts/re8_vr.lua`
- `scripts/utility/RE8.lua`

What changed:

- The C++ VR mod exposes these Lua methods:
  - `is_snap_turn_suppressed`
  - `set_snap_turn_suppressed`
  - `suppress_snap_turn_for`
- `scripts/re8_vr.lua` calls `vrmod:suppress_snap_turn_for(0.25)` while RE7 inventory is open.
- `scripts/re8_vr.lua` and `scripts/utility/RE8.lua` set snap-turn suppression from RE7/RE8 cutscene state.
- Initialization clears RE7 snap-turn suppression.

Why:

- In RE7 inventory examine, the right stick is needed to rotate items. Without suppression, that same stick movement also snap-turns the player.
- Cutscenes and scripted states should not trigger player locomotion comfort actions from incidental controller input.

### 6. Comfort Vignette State And Tuning

Purpose: reduce VR discomfort during movement, turning, and snap turns.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`
- `release/re2_fw_config.txt`

What changed:

- Added settings:
  - `VR_ComfortVignette`
  - `VR_ComfortVignetteStrength`
  - `VR_ComfortVignetteRange`
  - `VR_ComfortVignetteFadeIn`
  - `VR_ComfortVignetteFadeOut`
  - `VR_ComfortVignetteBeginAngle`
  - `VR_ComfortVignetteEndAngle`
- Added runtime state:
  - `m_comfort_vignette_amount`
  - `m_last_comfort_vignette_target`
  - `m_last_comfort_vignette_update`
  - `m_comfort_vignette_applied`
  - `m_cutscene_vignette_active`
  - cached post-effect/tone-mapping controller pointers
- Added `VR_ComfortVignetteCutscenes`, defaulting enabled.
- Added Lua-facing `set_cutscene_vignette_active`.
- `update_action_states` now calls `update_comfort_vignette`.
- Vignette amount is driven by:
  - left-stick movement magnitude
  - right-stick turn input
  - a short full-strength pulse after snap turn
  - cutscene state when cutscene vignette is enabled
- The fade-in behavior accelerates on sudden movement.
- Config load now forces or repairs known-good vignette defaults.
- The VR settings UI now shows movement/turn vignette controls, the cutscene vignette toggle, and current vignette amount.

Why:

- Stick movement and snap turning can be uncomfortable without peripheral dimming.
- The branch needs comfort settings to be enabled and tuned by default for the packaged RE7 build.
- Cutscenes can still create uncomfortable motion or camera transitions, so the current worktree lets Lua request a full comfort vignette while the game is in a cutscene-like state.

### 7. Engine Tone-Mapping Vignette Application

Purpose: apply the comfort vignette through RE Engine post-effect/tone-mapping parameters where available.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`

What changed:

- Added `apply_comfort_vignette`.
- It finds `posteffect.PostEffectController` and `posteffect.ToneMapController`.
- It updates available tone-mapping params to enable/disable kerare-style vignetting.
- It computes begin/end angles from configured range and current vignette amount.

Why:

- RE Engine already has post-effect parameters that can express vignette-like behavior.
- Using the engine parameters gives the mod another path to make movement comfort visible.

### 8. D3D11 Stereo Comfort Vignette Rendering

Purpose: draw a reliable per-eye comfort vignette directly into the VR backbuffer.

Files changed:

- `src/mods/vr/D3D11Component.cpp`
- `src/mods/vr/D3D11Component.hpp`

What changed:

- Added `d3dcompiler` usage and linked `d3dcompiler`.
- Added D3D11 resources for:
  - vertex shader
  - pixel shader
  - constant buffer
  - blend state
- `on_frame` now calls `draw_comfort_vignette` before submitting/copying the backbuffer.
- Added `setup_comfort_vignette` to compile a fullscreen triangle shader pair and create render state.
- Added `draw_comfort_vignette` to render a black alpha-blended edge mask.
- The shader constants include vignette amount, inner/outer width, aspect, and which eye is being drawn.
- D3D11 reset now releases vignette resources.

Why:

- Engine tone-mapping alone was not enough for a dependable stereo comfort effect.
- Drawing into the D3D11 backbuffer makes the comfort vignette visible in the VR output path.

### 9. RE7 UI Distance, Scale, And Near Clip Fixes

Purpose: keep RE7 UI readable and physically comfortable in VR.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`
- `release/re2_fw_config.txt`

What changed:

- Added RE7-specific constants:
  - minimum near Z of `0.1`
  - default 2D UI distance of `1.5`
  - minimum world UI distance of `1.0`
- The camera near clip is clamped to the RE7 minimum.
- 2D UI scale and world UI scale are clamped to smaller RE7-safe ranges.
- The 2D UI distance slider default/minimum changed from `1.0`/`0.01` to `1.5`/`0.5`.
- World-space UI placement now handles near-zero camera deltas safely and pushes UI out to a minimum distance when it would be too close.
- Config load repairs out-of-range RE7 UI values.

Why:

- RE7 UI elements could be too close, too large, or clipped in VR.
- The changes reduce eye strain and prevent UI surfaces from landing inside or too near the viewer.

### 10. RE7 Game-Over Comfort Suppression

Purpose: make the RE7 "You are dead" menu visible without the uncomfortable stereo scene/post-effect backdrop.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`

What changed:

- Added RE7-only game-over GUI detection by name patterns such as `GameOver`, `YouAreDead`, and `DeadMenu`.
- Added detection by RE7 GUI component types:
  - `gui.GameOverBehavior`
  - `gui.GameOverFadeBehavior`
  - `gui.GameOverSceneTimelineBehavior`
- When a game-over GUI element is seen, the mod opens a short rolling suppression window.
- During that window, the scene and post-effect render layer hooks return early.
- Current worktree edits also detect likely game-over capture/backdrop GUI names such as `Back`, `BG`, `Background`, `Capture`, `Screenshot`, `ImagePlane`, `Blur`, `Filter`, and `SceneTimeline`.
- Current worktree edits also detect backdrop/capture components such as `via.gui.BlurFilter`, `via.render.Mesh`, and `gui.GameOverSceneTimelineBehavior`.
- When the suppression window is active, matching game-over capture/backdrop GUI elements are skipped from `on_pre_gui_draw_element`.

Why:

- The game-over UI was visible over an uncomfortable or visually broken stereo background.
- Suppressing the background layers lets the menu remain legible and more comfortable.
- Skipping capture/backdrop GUI elements narrows the suppression to the specific visual pieces that create the bad stereo backdrop while preserving the foreground death menu.

### 11. Cutscene State Bridge From Lua

Purpose: let RE7/RE8 Lua-side cutscene detection drive C++ VR comfort behavior.

Files changed:

- `scripts/utility/RE7.lua`
- `scripts/utility/RE8.lua`
- `src/mods/VR.cpp`
- `src/mods/VR.hpp`
- `release/re2_fw_config.txt`

What changed:

- `scripts/utility/RE7.lua` adds `set_vr_cutscene_vignette` and calls it when RE7 cutscene state is initialized or changes.
- `scripts/utility/RE8.lua` generalizes its previous RE7 snap-turn suppression helper into `set_vr_cutscene_state`.
- `set_vr_cutscene_state` still suppresses RE7 snap turn during cutscenes, and now also calls `vrmod:set_cutscene_vignette_active`.
- C++ exposes `set_cutscene_vignette_active` to Lua and stores the state in `m_cutscene_vignette_active`.
- `update_comfort_vignette` treats active cutscene vignette as a full-strength vignette target when `VR_ComfortVignetteCutscenes` is enabled.
- `release/re2_fw_config.txt` enables `VR_ComfortVignetteCutscenes=true`.

Why:

- Lua already has better game-specific knowledge of cutscene state.
- Passing that state into C++ lets the renderer apply comfort treatment during scripted camera motion without guessing from controller input alone.

### 12. REFramework Menu Window Usability

Purpose: make the in-headset REFramework menu less cramped and less prone to accidental interaction issues.

Files changed:

- `src/REFramework.cpp`
- `src/mods/bindings/ImGui.cpp`

What changed:

- The main REFramework menu default width increased from `300` to `432`.
- The main window now has size constraints from `360x300` to `900x900`.
- The main window is created with flags preventing move, collapse, and mouse-wheel scrolling.
- Lua-created ImGui windows and child windows now strip horizontal scrollbar flags.

Why:

- The old menu was too narrow for VR use.
- Horizontal scroll/panning caused by controller interaction made the menu harder to use in headset.

### 13. OpenVR Overlay Menu Control

Purpose: make the OpenVR overlay menu open, close, and interact reliably with controllers.

Files changed:

- `src/mods/vr/OverlayComponent.cpp`
- `src/mods/vr/OverlayComponent.hpp`

What changed:

- Added overlay/menu state:
  - `m_force_show_ui`
  - `m_was_menu_combo_down`
  - `m_suppress_hand_open_until_clear`
  - `m_overlay_mouse_down`
  - `m_overlay_shown`
- Overlay state is fully reset on OpenVR initialization and D3D reset.
- The overlay is no longer shown immediately during initialization; it is shown once the runtime is ready.
- Smooth scroll events are disabled and any smooth scroll input is zeroed.
- Overlay flags now hide it from the dashboard tab, keep it from being visible in dashboard, and disable interaction until needed.
- Raw trigger state is read directly from OpenVR controller state as a fallback to action state.
- A controller trigger combo toggles the menu open/closed.
- Overlay interaction is enabled only when the menu is open.
- Hand-hover auto-open is suppressed until the controller clears after a close.

Why:

- The old overlay could pan unexpectedly, appear too early, or reopen immediately from hand intersection.
- Controller shortcuts needed to work even when action state was unreliable.

### 14. OpenVR Startup Dashboard Guard

Purpose: avoid startup dashboard events causing unwanted pause/menu behavior.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/vr/runtimes/OpenVR.cpp`
- `src/mods/vr/runtimes/OpenVR.hpp`

What changed:

- `OpenVR` now tracks `initialized_at`.
- `OpenVR` exposes `is_startup_pause_grace_period`, true before first poses or within five seconds of init.
- `consume_events` ignores `VREvent_DashboardActivated` during the startup grace period.
- RE-style input mapping suppresses system-button pause handling during the startup grace period.
- `initialize_openvr` temporarily disables SteamVR's `StartDashboardFromAppLaunch` setting while initializing and restores it afterward.

Why:

- SteamVR dashboard behavior during app startup could produce unwanted pause/menu activation before the VR session was actually ready.

### 15. OpenXR Menu Shortcut And Controller Compatibility

Purpose: make the OpenXR / VDXR path usable without relying on the OpenVR overlay behavior.

Files changed:

- `src/mods/VR.cpp`
- `src/mods/VR.hpp`
- `scripts/re8_vr.lua`
- `src/mods/vr/runtimes/OpenXR.hpp`

What changed:

- Added `update_openxr_menu_shortcut`, called from `update_action_states`.
- In OpenXR mode, pressing both controller triggers toggles the REFramework UI.
- RE2/RE3/RE7-style input mapping treats OpenXR left weapon-dial action as left trigger.
- `scripts/re8_vr.lua` treats left weapon dial as active left trigger and explicitly sets analog/button trigger state.
- `OpenXR.hpp` only changed by newline normalization in the final diff.

Why:

- The OpenXR path does not use the same overlay interaction path as OpenVR.
- RE7 expects certain left-trigger/weapon-dial behavior for inventory and item controls, so OpenXR actions need to map into that shape.

### 16. API Typing Cleanup

Purpose: make the SDK accessor return type explicit.

Files changed:

- `include/reframework/API.hpp`

What changed:

- `sdk()` changed from `inline const auto sdk() const` to `inline const REFrameworkSDKData* sdk() const`.
- The file ending was normalized with a final newline.

Why:

- This clarifies the API contract and avoids relying on `auto` deduction for the SDK pointer return type.

### 17. Working Memory Documentation

Purpose: preserve what has been learned about the working RE7 TDB49 build and avoid repeating broken paths.

Files changed:

- `MEMORY.md`

What changed:

- Added a branch memory document covering:
  - what this fork is for
  - the branch/release truth
  - why the DX11 non-RT / TDB49 line matters
  - known-bad newer universal/v2 outcomes
  - working build commands
  - game deploy and backup steps
  - working artifact details
  - tested comfort/controller behavior
  - inventory snap-turn conflict and fix
  - game-over comfort direction
  - follow-up observation that RE7 game-over discomfort also involves a captured/frozen last-frame backdrop and should suppress likely capture/backdrop GUI elements

Why:

- The branch depends on a specific working recipe. Capturing that context makes the code changes maintainable and the release process repeatable.

## File-By-File Inventory

| File | Diff role | What changed |
| --- | --- | --- |
| `.github/workflows/dev-release.yml` | Packaging workflow | Replaced multi-game artifact workflow with RE7-only build, package, and upload flow. |
| `.gitignore` | Local build hygiene | Added `build_v1_2*/` ignore pattern. |
| `CMakeLists.txt` | Build output layout | Added post-build copy commands for OpenVR/OpenXR loader DLLs for every game target; current worktree uses `${CMKR_TARGET}` in those generated blocks. |
| `MEMORY.md` | Documentation | Added working RE7 TDB49 branch memory, build recipe, deploy notes, known issue notes, and follow-up game-over backdrop suppression note. |
| `cmake.toml` | Build template | Added cmkr template post-build copy hook for runtime loader DLLs. |
| `dev/package-re7-tdb49.ps1` | Packaging script | Added script that stages DLLs, configs, scripts, revision metadata, and creates `RE7_TDB49.zip`. |
| `include/reframework/API.hpp` | API cleanup | Made `sdk()` return `const REFrameworkSDKData*` explicitly and normalized final newline. |
| `release/re2_fw_config.txt` | Release config | Added REFramework config defaults for VR comfort, cutscene comfort vignette, snap turning, UI scale/distance, and rendering flags. |
| `release/re7_config.ini` | Release config | Added tested RE7 game config for the packaged build. |
| `scripts/re8_vr.lua` | Input behavior | Suppresses snap turn in RE7 inventory/cutscenes and treats weapon dial as left trigger where needed. |
| `scripts/utility/RE7.lua` | Cutscene state bridge | Sends RE7 cutscene state to C++ so cutscenes can trigger comfort vignette. |
| `scripts/utility/RE8.lua` | Input/cutscene state bridge | Sends cutscene state to C++ for RE7 snap-turn suppression and cutscene comfort vignette. |
| `src/REFramework.cpp` | Menu usability | Enlarges/constrains the main menu and prevents move/collapse/mouse-wheel scroll. |
| `src/mods/VR.cpp` | Main VR behavior | Adds snap turn, comfort vignette logic, cutscene vignette state, OpenXR menu shortcut, RE7 UI fixes, game-over suppression/backdrop skipping, OpenVR startup guard, and controller compatibility. |
| `src/mods/VR.hpp` | Main VR declarations | Adds snap-turn/vignette/cutscene settings, state, and helper declarations; changes default UI distance. |
| `src/mods/bindings/ImGui.cpp` | Menu usability | Strips horizontal scrollbar flags from Lua-created ImGui windows and children. |
| `src/mods/vr/D3D11Component.cpp` | Comfort rendering | Adds D3D11 shader setup and draw path for stereo comfort vignette. |
| `src/mods/vr/D3D11Component.hpp` | Comfort rendering state | Adds shader, constant buffer, blend state members and vignette helper declarations. |
| `src/mods/vr/OverlayComponent.cpp` | OpenVR menu overlay | Refines overlay init, visibility, interaction, controller combo, raw trigger fallback, and scroll suppression. |
| `src/mods/vr/OverlayComponent.hpp` | OpenVR menu overlay state | Adds overlay/menu state flags used by the new interaction behavior. |
| `src/mods/vr/runtimes/OpenVR.cpp` | Startup behavior | Ignores dashboard activation during startup grace period. |
| `src/mods/vr/runtimes/OpenVR.hpp` | Startup behavior | Adds initialization timestamp and startup grace-period helper. |
| `src/mods/vr/runtimes/OpenXR.hpp` | Formatting only | Final diff shows newline normalization only. |

## Notes

- The inventory above intentionally avoids commit chronology. It groups the final code diff by the user-facing or build-facing purpose of each change.
- I used `master...HEAD` for "changes not in master" because it isolates work introduced on this branch. A literal `git diff master` is much larger in this repository because `master` has moved onto a newer upstream line, so it includes broad upstream/base divergence that is not part of this RE7 TDB49 change set.
- `release/re7_config.ini` contains a full captured game config, including display/adapter sections from the tested machine. That appears intentional for reproducing the working package, but it is hardware-specific data in the release payload.
- Once this document is committed, future releases should treat it as part of the branch's release metadata and update it alongside release-facing code/config changes.
