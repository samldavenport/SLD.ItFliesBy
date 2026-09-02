$ProjectRoot = Split-Path -Parent $PSScriptRoot 

$Win32File        = Join-Path $ProjectRoot "src\win32\win32-main.cpp"
$Win32Command     = @(
    "cl.exe"
    $File
    "/Fo:" + @(Join-Path $ProjectRoot "build\debug\obj\ItFliesBy.obj")  
    "/I"   + @(Join-Path $ProjectRoot "src\win32")
    "/I"   + @(Join-Path $ProjectRoot "src\game")
    "/I"   + @(Join-Path $ProjectRoot "include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Core\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Opengl\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Math\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Strings\include")
    "/I"   + @(Join-Path $ProjectRoot "vcpkg_installed\x64-windows\include")
    "/nologo"             # startup banner disabled
    "/c"                  # compile without linking
    "/MD"                 # link against multithreaded runtime library (MSVCRT.dll)
    "/Z7"                 # generate debug info
    "/EHs-"               # disable exception handling
    "/std:c++17"          # c++17 standard mode
    "/Od"                 # disable optimizations
    "/D_HAS_EXCEPTIONS=0" # disable exceptions for STL and CRT
    "/DSLD_DLL_EXPORT"
    "/DIFB_ENGINE_DLL_EXPORT"
    "/DIFB_API_DLL_EXPORT"
) -join " "

$EngFile        = Join-Path $ProjectRoot "src\engine\eng.cpp"
$EngCommand     = @(
    "cl.exe"
    $File
    "/Fo:" + @(Join-Path $ProjectRoot "build\debug\obj\ItFliesBy.obj")  
    "/I"   + @(Join-Path $ProjectRoot "include")
    "/I"   + @(Join-Path $ProjectRoot "src\collections")
    "/I"   + @(Join-Path $ProjectRoot "src\components")
    "/I"   + @(Join-Path $ProjectRoot "src\engine")
    "/I"   + @(Join-Path $ProjectRoot "src\entities")
    "/I"   + @(Join-Path $ProjectRoot "src\files")
    "/I"   + @(Join-Path $ProjectRoot "src\gui")
    "/I"   + @(Join-Path $ProjectRoot "src\json")
    "/I"   + @(Join-Path $ProjectRoot "src\map")
    "/I"   + @(Join-Path $ProjectRoot "src\memory")
    "/I"   + @(Join-Path $ProjectRoot "src\physics")
    "/I"   + @(Join-Path $ProjectRoot "src\quads")
    "/I"   + @(Join-Path $ProjectRoot "src\renderer")
    "/I"   + @(Join-Path $ProjectRoot "src\tiled")
    "/I"   + @(Join-Path $ProjectRoot "src\win32")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Core\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.OpenGL\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Math\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Strings\include")
    "/I"   + @(Join-Path $ProjectRoot "vcpkg_installed\x64-windows\include")
    "/nologo"             # startup banner disabled
    "/c"                  # compile without linking
    "/MD"                 # link against multithreaded runtime library (MSVCRT.dll)
    "/Z7"                 # generate debug info
    "/EHs-"               # disable exception handling
    "/std:c++17"          # c++17 standard mode
    "/Od"                 # disable optimizations
    "/D_HAS_EXCEPTIONS=0" # disable exceptions for STL and CRT
    "/DSLD_DLL_EXPORT"
    "/DIFB_ENGINE_DLL_EXPORT"
    "/DIFB_API_DLL_EXPORT"
) -join " "

$CompileCommands  = @()
$CompileCommands += [PSCustomObject]@{
    directory = $ProjectRoot  
    command   = $EngCommand
    file      = $EngFile 
}
$CompileCommands += [PSCustomObject]@{
    directory = $ProjectRoot  
    command   = $Win32Command
    file      = $Win32File 
}

ConvertTo-Json -InputObject $CompileCommands -Depth 10 |
    Set-Content "compile_commands.json"
