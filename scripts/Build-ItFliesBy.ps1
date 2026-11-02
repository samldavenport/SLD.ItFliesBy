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
# the VS developer shell points it at the copy bundled with Visual Studio, which needs
# a baseline that vcpkg.json doesn't have, so that copy is never used
if ((-not $env:VCPKG_ROOT) -or ($env:VCPKG_ROOT -like "*\Microsoft Visual Studio\*")) {
    $Vcpkg = Get-Command vcpkg -All -ErrorAction SilentlyContinue |
        Where-Object { $_.Source -notlike "*\Microsoft Visual Studio\*" } |
        Select-Object -First 1
    if (-not $Vcpkg) {
        throw "vcpkg was not found on PATH, outside of the copy bundled with Visual Studio"
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
