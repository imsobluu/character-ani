# Build script for gen3dart standalone OpenGL project
#
# Prerequisites:
#   1. CMake must be in PATH (or set CMAKE_PATH environment variable)
#   2. Visual Studio 2022 with C++ workload installed

# Get script directory
$SCRIPT_DIR = $PSScriptRoot
if (-not $SCRIPT_DIR) { $SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path }

# CMake - use from CMAKE_PATH, user-provided D:\CMake install, or PATH
if ($env:CMAKE_PATH) {
    if (Test-Path $env:CMAKE_PATH -PathType Container) {
        $CMAKE_PATH = Join-Path $env:CMAKE_PATH "bin\cmake.exe"
    } else {
        $CMAKE_PATH = $env:CMAKE_PATH
    }
} elseif (Test-Path "D:\CMake\bin\cmake.exe") {
    $CMAKE_PATH = "D:\CMake\bin\cmake.exe"
} else {
    $CMAKE_PATH = (Get-Command cmake -ErrorAction SilentlyContinue).Source
}

if (-not (Test-Path $CMAKE_PATH)) {
    Write-Host "Error: CMake not found. Set CMAKE_PATH or install at D:\CMake." -ForegroundColor Red
    exit 1
}

$OUTPUT_DIR = "$SCRIPT_DIR\out"
$BIN_DIR = "$SCRIPT_DIR\bin"
$DLL_DIR = "$SCRIPT_DIR\dlls"

$GENERATOR = ""
$IS_MULTI_CONFIG = $false
$VSWHERE = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VSWHERE) {
    $VS_INSTALL = & $VSWHERE -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if ($LASTEXITCODE -eq 0 -and $VS_INSTALL) {
        $GENERATOR = "Visual Studio 17 2022"
        $IS_MULTI_CONFIG = $true
    }
}

if (-not $GENERATOR) {
    $MINGW_MAKE = (Get-Command mingw32-make -ErrorAction SilentlyContinue).Source
    $GXX = (Get-Command g++ -ErrorAction SilentlyContinue).Source
    if ($MINGW_MAKE -and $GXX) {
        $GENERATOR = "MinGW Makefiles"
    } else {
        Write-Host "Error: no supported build toolchain found (Visual Studio or MinGW)." -ForegroundColor Red
        exit 1
    }
}

Write-Host "Using CMake: $CMAKE_PATH" -ForegroundColor Gray
Write-Host "Using Generator: $GENERATOR" -ForegroundColor Gray
Write-Host ""

# Create output directories
New-Item -ItemType Directory -Force -Path $OUTPUT_DIR | Out-Null
New-Item -ItemType Directory -Force -Path $BIN_DIR | Out-Null

# Change to output directory
Push-Location $OUTPUT_DIR

# Ensure generator switches don't conflict with stale cache
if (Test-Path "$OUTPUT_DIR\CMakeCache.txt") {
    Remove-Item "$OUTPUT_DIR\CMakeCache.txt" -Force
}
if (Test-Path "$OUTPUT_DIR\CMakeFiles") {
    Remove-Item "$OUTPUT_DIR\CMakeFiles" -Recurse -Force
}

# Configure CMake
Write-Host "Configuring CMake..." -ForegroundColor Cyan
if ($IS_MULTI_CONFIG) {
    & $CMAKE_PATH -G $GENERATOR -A x64 -T v143 $SCRIPT_DIR
} else {
    & $CMAKE_PATH -G $GENERATOR $SCRIPT_DIR
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build the skeletal_animation project
Write-Host "Building skeletal_animation..." -ForegroundColor Cyan
if ($IS_MULTI_CONFIG) {
    & $CMAKE_PATH --build . --config Debug --target skeletal_animation
} else {
    & $CMAKE_PATH --build . --target skeletal_animation
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Copy shader files to bin directory
Write-Host "Copying shader files..." -ForegroundColor Cyan
Copy-Item "$SCRIPT_DIR\src\8.guest\2020\skeletal_animation\anim_model.vs" -Destination $BIN_DIR -Force
Copy-Item "$SCRIPT_DIR\src\8.guest\2020\skeletal_animation\anim_model.fs" -Destination $BIN_DIR -Force
Copy-Item "$SCRIPT_DIR\src\8.guest\2020\skeletal_animation\walk_area.vs" -Destination $BIN_DIR -Force
Copy-Item "$SCRIPT_DIR\src\8.guest\2020\skeletal_animation\walk_area.fs" -Destination $BIN_DIR -Force
Copy-Item "$SCRIPT_DIR\src\8.guest\2020\skeletal_animation\skybox.vs" -Destination $BIN_DIR -Force
Copy-Item "$SCRIPT_DIR\src\8.guest\2020\skeletal_animation\skybox.fs" -Destination $BIN_DIR -Force

# Copy runtime DLLs from dlls folder (if any)
if (Test-Path $DLL_DIR) {
    Write-Host "Copying runtime DLLs..." -ForegroundColor Cyan
    Copy-Item "$DLL_DIR\*.dll" -Destination $BIN_DIR -Force -ErrorAction SilentlyContinue
}

Write-Host "`nBuild complete!" -ForegroundColor Green
Write-Host "Executable: $BIN_DIR\skeletal_animation.exe" -ForegroundColor Yellow
Write-Host "`nTo run: cd '$BIN_DIR'; .\skeletal_animation.exe" -ForegroundColor Yellow
