# Agent Instructions

## Release Delta Manifest

Always keep `CHANGES_NOT_IN_MASTER.md` up to date when making changes that affect the release build, packaging, runtime behavior, configs, scripts, or any other difference from official REFramework `master`.

Before finishing release-relevant work, update the manifest so it accurately explains what changed from official `master`, why the change exists, and which files are involved.

## Upstream And Branch Baseline

`origin` is this fork (`tsilva/REFramework-chill`). `upstream` is the official [praydog/REFramework](https://github.com/praydog/REFramework) repository.

The RE7 release branch is intentionally based on the upstream `v1.2` / RE7 TDB49-era REFramework line, not current upstream `master`.

Do not pull, merge, or rebase current `upstream/master` into the RE7 release branch unless the user explicitly asks for a porting effort. Normal release work should preserve the v1.2/TDB49 baseline.

When upstream context is needed for comparison or release version metadata, fetch from official upstream, for example `git fetch upstream master --tags`, and use the merge base/source tag for identification only.
