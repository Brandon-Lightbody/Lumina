#!/usr/bin/env pwsh
param(
    [ValidateSet('vs-debug', 'clang-debug', 'ninja-debug', 'release')]
    [string]$Preset = 'vs-debug'
)

# Map preset names to configure presets
$ConfigMap = @{
    'vs-debug' = 'windows-debug'
    'clang-debug' = 'clang-debug'
    'ninja-debug' = 'ninja-debug'
    'release' = 'release'
}

$ConfigPreset = $ConfigMap[$Preset]

# Update submodules
Write-Host "Updating submodules..." -ForegroundColor Cyan
git submodule update --init --recursive

# Initialize NRI submodule specifically
Write-Host "Initializing NRI submodule..." -ForegroundColor Cyan
Push-Location "Lumina/Runtime/deps/NRI"
git submodule update --init --recursive
Pop-Location

# Configure and build
Write-Host "Configuring $Preset build..." -ForegroundColor Cyan
cmake --preset $ConfigPreset

Write-Host "Building project..." -ForegroundColor Cyan
cmake --build --preset $Preset

# Post-build info
Write-Host "`nBuild completed!" -ForegroundColor Green
$OutputDir = "build/$($ConfigPreset.ToLower())/bin"
if (Test-Path $OutputDir) {
    Write-Host "Output directory: $(Resolve-Path $OutputDir)"
    
    # Check for ASan reports (only for debug builds)
    if ($Preset -match "debug") {
        $AsanReport = Join-Path (Resolve-Path $OutputDir) "asan_report.txt"
        Write-Host "ASan report path: $AsanReport"
    }
}