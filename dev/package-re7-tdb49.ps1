param(
    [string] $BuildDir = "build",
    [string] $Configuration = "Release",
    [string] $OutputDir = "build\release",
    [string] $PackageName = "RE7_TDB49.zip"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = if ([System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir } else { Join-Path $repoRoot $BuildDir }
$outputRoot = if ([System.IO.Path]::IsPathRooted($OutputDir)) { $OutputDir } else { Join-Path $repoRoot $OutputDir }
$stageRoot = Join-Path $outputRoot "RE7_TDB49"
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
    "target=RE7_TDB49 OpenXR/OpenVR",
    "package=$PackageName"
) | Set-Content -LiteralPath (Join-Path $stageRoot "reframework_revision.txt") -Encoding ASCII

Compress-Archive -Path (Join-Path $stageRoot "*") -DestinationPath $zipPath -Force
Write-Host "Wrote $zipPath"
