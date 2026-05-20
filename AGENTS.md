# AGENTS.md

This fork is a private integration branch for REFramework quality-of-life work focused on Resident Evil 7 VR, Virtual Desktop, VDXR/OpenXR, and Quest 3.

The priority is practical usability for this setup. For now, it is acceptable if changes are not generic enough for upstream or if they risk behavior in other games. We may broaden support later, but the current goal is to make the RE7 + VD + VDXR + Quest 3 experience better and easier to distribute.

## Branch Policy

- `tsilva/dev` is the single integration branch for our work.
- All finished and verified work must end up merged into `tsilva/dev`.
- Never commit or merge our patch work into `master`.
- `master` should remain aligned with upstream `origin/master`.
- Temporary feature branches are allowed, but they should use a `tsilva/` prefix, for example:
  - `tsilva/re7-menu-shortcut`
  - `tsilva/vdxr-vignette-tuning`
  - `tsilva/quest3-input-polish`
- After a feature branch is working and verified, merge it into `tsilva/dev`.
- Once a feature branch has been merged into `tsilva/dev`, delete the temporary branch unless there is a specific reason to keep it.

## Current Direction

This fork currently carries local patches for RE7-focused VR comfort, menu behavior, and input quality-of-life work. Examples of changes we have added include:

- Snap turning for RE7/VR with configurable angle and threshold.
- Smooth turn speed scaling for right-stick turning.
- Movement, smooth-turn, and snap-turn comfort vignette support.
- D3D11 eye-texture vignette rendering so OpenXR/VDXR receives the comfort mask.
- OpenVR hand menu targeting improvements.
- OpenVR menu open/close shortcut handling.
- OpenXR both-trigger menu shortcut handling.
- REFramework menu sizing and scroll behavior tweaks for VR overlay use.
- Lua ImGui horizontal scrollbar suppression to reduce accidental menu panning/layout damage.
- RE7-specific UI scale and distance defaults for VR readability.

When making changes, prefer small, reviewable commits with names that describe the user-facing behavior. This branch is allowed to be pragmatic, but avoid unrelated refactors unless they are necessary for the patch.

## Syncing Upstream

Upstream is `origin`, and upstream's main development branch is `origin/master`.

To bring upstream changes into our branch:

```powershell
cd C:\Users\engti\Desktop\REFramework-clean
git switch dev
git fetch origin
git merge origin/master
```

If there are conflicts, resolve them by preserving our RE7/VD/VDXR/Quest 3 behavior unless upstream has clearly replaced it with a better equivalent. After resolving conflicts:

```powershell
git status
git add <resolved-files>
git commit
git push tsilva dev
```

Do not rebase published `tsilva/dev` unless everyone using the branch agrees. Prefer merge commits from `origin/master` so deployed builds and tags remain easy to trace.

## Feature Workflow

For larger changes, develop on a temporary `tsilva/` branch:

```powershell
git switch dev
git fetch origin
git merge origin/master
git switch -c tsilva/my-feature
```

Work, commit, and test on the feature branch. When the change is verified:

```powershell
git switch dev
git merge --no-ff tsilva/my-feature
git push tsilva dev
```

Then delete the feature branch locally and remotely if it is no longer needed:

```powershell
git branch -d tsilva/my-feature
git push tsilva --delete tsilva/my-feature
```

## Release Notes

Tags and release artifacts should be cut from `tsilva/dev`, not from `master` or a temporary feature branch. Include enough detail in release notes to identify:

- The upstream commit merged into `dev`.
- The local patch version.
- The target setup, especially RE7, Virtual Desktop, VDXR/OpenXR, and Quest 3.
- Any known game compatibility risks.

