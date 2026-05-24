param(
    [string] $BuildDir = "build",
    [string] $Configuration = "Release",
    [string] $OutputDir = "build\release",
    [string] $PackageName = "",
    [string] $UpstreamBranch = "upstream/master"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = if ([System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir } else { Join-Path $repoRoot $BuildDir }
$outputRoot = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $repoRoot $OutputDir }

function Invoke-GitValue {
    param([string[]] $Arguments)

    $result = & git -C $repoRoot @Arguments

    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed"
    }

    return ($result | Select-Object -First 1).Trim()
}

function Get-ExistingUpstreamBranch {
    param([string] $PreferredBranch)

    & git -C $repoRoot rev-parse --verify --quiet $PreferredBranch | Out-Null

    if ($LASTEXITCODE -eq 0) {
        return $PreferredBranch
    }

    & git -C $repoRoot rev-parse --verify --quiet "origin/master" | Out-Null

    if ($LASTEXITCODE -eq 0) {
        return "origin/master"
    }

    & git -C $repoRoot rev-parse --verify --quiet "master" | Out-Null

    if ($LASTEXITCODE -eq 0) {
        return "master"
    }

    throw "Could not find upstream branch '$PreferredBranch' or fallback branches 'origin/master'/'master'"
}

function Get-ForkBuildVersion {
    $exactTag = & git -C $repoRoot tag --points-at HEAD | Select-Object -First 1

    if ($null -ne $exactTag) {
        $exactTag = $exactTag.Trim()
    }

    if (![string]::IsNullOrWhiteSpace($exactTag) -and $exactTag -match "(v[0-9]+(?:[._][0-9]+)*)$") {
        return $matches[1]
    }

    $description = Invoke-GitValue @("describe", "--tags", "--long", "--always", "HEAD")

    if ($description -match "(v[0-9]+(?:[._][0-9]+)*)(-[0-9]+-g[0-9a-f]+)?$") {
        return "$($matches[1])$($matches[2])"
    }

    $shortRevision = Invoke-GitValue @("rev-parse", "--short", "HEAD")
    return "g$shortRevision"
}

$resolvedUpstreamBranch = Get-ExistingUpstreamBranch $UpstreamBranch
$upstreamMergeBase = Invoke-GitValue @("merge-base", $resolvedUpstreamBranch, "HEAD")
$upstreamSourceVersion = Invoke-GitValue @("describe", "--tags", "--abbrev=0", "--match", "v*", $upstreamMergeBase)
$forkBuildVersion = Get-ForkBuildVersion
$buildVersion = "$upstreamSourceVersion-RE7-antipuke-$forkBuildVersion"

if ([string]::IsNullOrWhiteSpace($PackageName)) {
    $PackageName = "$buildVersion.zip"
}

$packageBaseName = [System.IO.Path]::GetFileNameWithoutExtension($PackageName)
$stageRoot = Join-Path $outputRoot $packageBaseName
$zipPath = Join-Path $outputRoot $PackageName

$dinput = Join-Path $buildRoot "bin\RE7\dinput8.dll"
$openxr = Join-Path $buildRoot "bin\RE7\openxr_loader.dll"
$openxrFallback = Join-Path $buildRoot "_deps\openxr-build\src\loader\$Configuration\openxr_loader.dll"
$openvr = Join-Path $buildRoot "bin\RE7\openvr_api.dll"
$openvrFallback = Join-Path $buildRoot "bin\openvr_api.dll"
$scripts = Join-Path $repoRoot "scripts"
$config = Join-Path $repoRoot "release\re2_fw_config.txt"
$gameConfig = Join-Path $repoRoot "release\re7_config.ini"

if (!(Test-Path -LiteralPath $openxr) -and (Test-Path -LiteralPath $openxrFallback)) {
    $openxr = $openxrFallback
}

if (!(Test-Path -LiteralPath $openvr) -and (Test-Path -LiteralPath $openvrFallback)) {
    $openvr = $openvrFallback
}

foreach ($required in @($dinput, $openxr, $openvr, $scripts, $config, $gameConfig)) {
    if (!(Test-Path -LiteralPath $required)) {
        throw "Required package input is missing: $required"
    }
}

Remove-Item -LiteralPath $stageRoot -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $zipPath -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $stageRoot | Out-Null
New-Item -ItemType Directory -Path (Join-Path $stageRoot "reframework\autorun") | Out-Null

Copy-Item -LiteralPath $dinput -Destination (Join-Path $stageRoot "dinput8.dll") -Force
Copy-Item -LiteralPath $openxr -Destination (Join-Path $stageRoot "openxr_loader.dll") -Force
Copy-Item -LiteralPath $openvr -Destination (Join-Path $stageRoot "openvr_api.dll") -Force
Copy-Item -LiteralPath $config -Destination (Join-Path $stageRoot "re2_fw_config.txt") -Force
Copy-Item -LiteralPath $gameConfig -Destination (Join-Path $stageRoot "re7_config.ini") -Force
Copy-Item -Path (Join-Path $scripts "*") -Destination (Join-Path $stageRoot "reframework\autorun") -Recurse -Force

$revision = git -C $repoRoot rev-parse HEAD
$branch = git -C $repoRoot branch --show-current
@(
    "source=tsilva/REFramework",
    "branch=$branch",
    "commit=$revision",
    "version=$buildVersion",
    "upstream_branch=$resolvedUpstreamBranch",
    "upstream_merge_base=$upstreamMergeBase",
    "upstream_source_version=$upstreamSourceVersion",
    "fork_build_version=$forkBuildVersion",
    "target=RE7_TDB49 OpenXR/OpenVR",
    "package=$PackageName"
) | Set-Content -LiteralPath (Join-Path $stageRoot "reframework_revision.txt") -Encoding ASCII

Compress-Archive -Path (Join-Path $stageRoot "*") -DestinationPath $zipPath -Force
Write-Host "Wrote $zipPath"
