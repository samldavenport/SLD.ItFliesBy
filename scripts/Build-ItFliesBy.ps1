param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug",

    # remove all generated files instead of building
    [switch]$Clean
)

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Preset      = $Config.ToLower()

if ($Clean) {
    $Generated = @(
        Join-Path $ProjectRoot "build"
        Join-Path $ProjectRoot "compile_commands.json"
    )
    foreach ($Path in $Generated) {
        if (Test-Path $Path) {
            Remove-Item $Path -Recurse -Force
        }
    }
    return
}

# the cmake presets locate the vcpkg toolchain through VCPKG_ROOT
if (-not $env:VCPKG_ROOT) {
    $Vcpkg = Get-Command vcpkg -ErrorAction SilentlyContinue
    if (-not $Vcpkg) {
        throw "VCPKG_ROOT is not set and vcpkg was not found on PATH"
    }
    $env:VCPKG_ROOT = Split-Path -Parent $Vcpkg.Source
}

Push-Location $ProjectRoot
try {
    cmake --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed ($Preset)" }

    cmake --build --preset $Preset
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed ($Preset)" }

    Copy-Item "build\$Preset\compile_commands.json" "compile_commands.json"
}
finally {
    Pop-Location
}
