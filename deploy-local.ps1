# deploy-local.ps1
# Copies the local build output to the MO2 BodySlide Studio mod folder,
# mirroring what the GitHub Actions workflow packages (EXEs + res/ + lang/).
#
# Config XML files (Config.xml, RefTemplates.xml, BodySlide.xml, etc.) are
# intentionally NOT copied — those live in MO2 with custom paths and settings.

$ErrorActionPreference = "Stop"

$RepoRoot  = $PSScriptRoot
$BuildDir  = Join-Path $RepoRoot "build\windows-x64-release\Release"
$Dest      = "F:\ModManager\Mo2-Starfield\Data\mods\BodySlide Studio\Tools\Bodyslide"

# ── Verify build outputs exist ──────────────────────────────────────────────
$exes = @(
    Join-Path $BuildDir "BodySlide x64.exe"
    Join-Path $BuildDir "OutfitStudio x64.exe"
)
foreach ($exe in $exes) {
    if (-not (Test-Path $exe)) {
        Write-Error "Build output not found: $exe`nRun cmake --build first."
    }
}

Write-Host "Deploying to: $Dest" -ForegroundColor Cyan

# ── EXEs ────────────────────────────────────────────────────────────────────
foreach ($exe in $exes) {
    $name = Split-Path $exe -Leaf
    Write-Host "  EXE  $name"
    Copy-Item $exe (Join-Path $Dest $name) -Force
}

# ── PDB files (optional, skip silently if absent) ────────────────────────────
$pdbs = Get-ChildItem $BuildDir -Filter "*.pdb" -ErrorAction SilentlyContinue
foreach ($pdb in $pdbs) {
    Write-Host "  PDB  $($pdb.Name)"
    Copy-Item $pdb.FullName (Join-Path $Dest $pdb.Name) -Force
}

# ── res/ folder (XRC, shaders, images, skeletons) ───────────────────────────
Write-Host "  DIR  res\"
Copy-Item (Join-Path $RepoRoot "res") $Dest -Recurse -Force

# ── lang/ folder ─────────────────────────────────────────────────────────────
Write-Host "  DIR  lang\"
Copy-Item (Join-Path $RepoRoot "lang") $Dest -Recurse -Force

Write-Host ""
Write-Host "Done." -ForegroundColor Green
