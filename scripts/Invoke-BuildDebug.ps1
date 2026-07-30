$ProjectRoot = Split-Path -Parent $PSScriptRoot 

$EngSrcFile    = Join-Path $ProjectRoot "src\engine\eng.cpp"
$EngCompile = @(
    "cl.exe"
    $EngSrcFile
    "/Fo:" + @(Join-Path $ProjectRoot "build\debug\obj\ItFliesBy.Engine.obj")  
    "/I"   + @(Join-Path $ProjectRoot "include")
    "/I"   + @(Join-Path $ProjectRoot "src\collections")
    "/I"   + @(Join-Path $ProjectRoot "src\components")
    "/I"   + @(Join-Path $ProjectRoot "src\engine")
    "/I"   + @(Join-Path $ProjectRoot "src\entities")
    "/I"   + @(Join-Path $ProjectRoot "src\files")
    "/I"   + @(Join-Path $ProjectRoot "src\gui")
    "/I"   + @(Join-Path $ProjectRoot "src\memory")
    "/I"   + @(Join-Path $ProjectRoot "src\physics")
    "/I"   + @(Join-Path $ProjectRoot "src\quads")
    "/I"   + @(Join-Path $ProjectRoot "src\renderer")
    "/I"   + @(Join-Path $ProjectRoot "src\win32")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Core\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.OpenGL\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Math\include")
    "/I"   + @(Join-Path $ProjectRoot "vcpkg_installed\x64-windows\include")
    "/nologo"             # startup banner disabled
    "/c"                  # compile without linking
    "/MD"                 # link against multithreaded runtime library (MSVCRT.dll)
    "/Z7"                 # generate debug info
    "/EHs-"               # disable exception handling
    "/std:c++17"          # c++17 standard mode
    "/Od"                 # disable optimizations
    "/D_HAS_EXCEPTIONS=0" # disable exceptions for STL and CRT
) -join " "

$EngLink = @(
    "link.exe"
    "/nologo"
    "/SUBSYSTEM:WINDOWS"
    "/DEBUG"
    "/DLL"
    "ItFliesBy.Engine.obj"
    "SLD.OpenGL.lib"
    "SLD.Core.lib"
    "user32.lib"
    "Gdi32.lib"
    "opengl32.lib"
    "glew32.lib"
    "imgui.lib"
    "/LIBPATH:build\debug\obj"
    "/LIBPATH:build\debug\lib"
    "/LIBPATH:vcpkg_installed/x64-windows/lib"
    "/OUT:build\debug\bin\ItFliesBy.Engine.dll"
    "/IMPLIB:build\debug\lib\ItFliesBy.Engine.lib"
) -join " "

$Win32SrcFile = Join-Path $ProjectRoot "src\win32\win32-main.cpp"
$Win32Compile = @(
    "cl.exe"
    $Win32SrcFile
    "/Fo:" + @(Join-Path $ProjectRoot "build\debug\obj\ItFliesBy.obj")  
    "/I"   + @(Join-Path $ProjectRoot "src\win32")
    "/I"   + @(Join-Path $ProjectRoot "include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Core\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Opengl\include")
    "/I"   + @(Join-Path $ProjectRoot "SLD.Math\include")
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
) -join " "

$Win32Link = @(
    "link.exe"
    "/nologo"
    "/SUBSYSTEM:WINDOWS"
    "/DEBUG"
    "/LIBPATH:build\debug\obj"
    "/LIBPATH:build\debug\lib"
    "/LIBPATH:vcpkg_installed\x64-windows\lib"
    "ItFliesBy.obj"
    "ItFliesBy.Engine.lib"
    "user32.lib"
    "kernel32.lib"
    "imgui.lib"
    "/OUT:build\debug\bin\ItFliesBy.exe"
) -join " "

& .\SLD.Core\scripts\Build-SLDCoreDebug.ps1

Invoke-Expression $EngCompile
Invoke-Expression $EngLink
Invoke-Expression $Win32Compile
Invoke-Expression $Win32Link

