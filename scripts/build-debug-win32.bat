@echo off
@echo off

pushd ..

::--------------------------------------------------------------------
:: BUILD DIRECTORIES
::--------------------------------------------------------------------

@set dir_bin= build\debug\bin
@set dir_lib= build\debug\lib
@set dir_obj= build\debug\obj

IF NOT EXIST %dir_bin% mkdir %dir_bin%
IF NOT EXIST %dir_lib% mkdir %dir_lib%
IF NOT EXIST %dir_obj% mkdir %dir_obj%

::--------------------------------------------------------------------
:: DEPENDENCIES 
::--------------------------------------------------------------------

robocopy vcpkg_installed\x64-windows\bin build\debug\bin *.dll > nul
robocopy assets\shaders\ build\debug\bin *.glsl > nul

::--------------------------------------------------------------------
:: SLD.Core 
::--------------------------------------------------------------------

@set core_cl_in=      SLD.Core\src\sld.cpp
@set core_cl_out=     /Fo:build\debug\obj\SLD.Core.obj
@set core_cl_include= /ISLD.Core\src /ISLD.Core\include /ISLD.Core\3rd-party
@set core_cl_flags=   /nologo /c /MD /LD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0

@set core_link_in=    SLD.Core.obj user32.lib
@set core_link_out=   /OUT:build\debug\bin\SLD.Core.dll /IMPLIB:build\debug\lib\SLD.Core.lib
@set core_link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib /LIBPATH:vcpkg_installed/x64-windows/lib
@set core_link_flags= /nologo /SUBSYSTEM:WINDOWS /DEBUG /DLL

@set core_cmd_cl=     cl.exe   %core_cl_in%      %core_cl_out%    %core_cl_include% %core_cl_flags%
@set core_cmd_link=   link.exe %core_link_flags% %core_link_path% %core_link_in%    %core_link_out%

call %core_cmd_cl%
call %core_cmd_link%

::--------------------------------------------------------------------
:: SLD.OpenGL 
::--------------------------------------------------------------------

@set gl_cl_in=      SLD.OpenGL\src\sld-opengl.cpp
@set gl_cl_out=     /Fo:build\debug\obj\SLD.OpenGL.obj
@set gl_cl_include= /ISLD.OpenGL\src /ISLD.OpenGL\include /ISLD.Core/include /Ivcpkg_installed/x64-windows/include
@set gl_cl_flags=   /nologo /c /MD /LD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0

@set gl_link_in=    SLD.OpenGL.obj user32.lib opengl32.lib gdi32.lib glew32.lib imgui.lib
@set gl_link_out=   /OUT:build\debug\bin\SLD.OpenGL.dll /IMPLIB:build\debug\lib\SLD.OpenGL.lib
@set gl_link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib /LIBPATH:vcpkg_installed/x64-windows/lib
@set gl_link_flags= /nologo /SUBSYSTEM:WINDOWS /DEBUG /DLL

@set gl_cmd_cl=     cl.exe   %gl_cl_in%      %gl_cl_out%    %gl_cl_include% %gl_cl_flags%
@set gl_cmd_link=   link.exe %gl_link_flags% %gl_link_path% %gl_link_in%    %gl_link_out%

call %gl_cmd_cl%
call %gl_cmd_link%

::--------------------------------------------------------------------
:: ENGINE 
::--------------------------------------------------------------------

@set eng_cl_in=      src\engine\eng.cpp
@set eng_cl_out=     /Fo:build\debug\obj\ItFliesBy.Engine.obj
@set eng_cl_include= /Iinclude /Isrc\collections /Isrc\components /Isrc\engine /Isrc\entities /Isrc\files /Isrc\gui /Isrc\memory /Isrc\quads /Isrc\renderer /Isrc\win32 /ISLD.Core\include /ISLD.OpenGL\include /ISLD.Math\include  /Ivcpkg_installed\x64-windows\include
@set eng_cl_flags=   /nologo /c /MD /LD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0

@set eng_link_in=    ItFliesBy.Engine.obj SLD.OpenGL.lib SLD.Core.lib user32.lib Gdi32.lib opengl32.lib glew32.lib imgui.lib
@set eng_link_out=   /OUT:build\debug\bin\ItFliesBy.Engine.dll /IMPLIB:build\debug\lib\ItFliesBy.Engine.lib
@set eng_link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib /LIBPATH:vcpkg_installed/x64-windows/lib
@set eng_link_flags= /nologo /SUBSYSTEM:WINDOWS /DEBUG /DLL

@set eng_cmd_cl=     cl.exe   %eng_cl_in%      %eng_cl_out%    %eng_cl_include% %eng_cl_flags%
@set eng_cmd_link=   link.exe %eng_link_flags% %eng_link_path% %eng_link_in%    %eng_link_out%

call %eng_cmd_cl%
call %eng_cmd_link%

::--------------------------------------------------------------------
:: WIN32 
::--------------------------------------------------------------------

@set win32_cl_in=      src\win32\win32-main.cpp
@set win32_cl_out=     /Fo:build\debug\obj\ItFliesBy.obj
@set win32_cl_include= /Iinclude /Isrc\win32 /ISLD.Core\include  /ISLD.OpenGL\include /ISLD.Math\include /Ivcpkg_installed\x64-windows\include
@set win32_cl_flags=   /nologo /c /MD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0

@set win32_link_in=    ItFliesBy.obj ItFliesBy.Engine.lib user32.lib kernel32.lib imgui.lib
@set win32_link_out=   /OUT:build\debug\bin\ItFliesBy.exe
@set win32_link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib /LIBPATH:vcpkg_installed\x64-windows\lib
@set win32_link_flags= /nologo /SUBSYSTEM:WINDOWS /DEBUG

@set win32_cmd_cl=     cl.exe   %win32_cl_in%      %win32_cl_out%    %win32_cl_include% %win32_cl_flags%
@set win32_cmd_link=   link.exe %win32_link_flags% %win32_link_path% %win32_link_in%    %win32_link_out%

call %win32_cmd_cl%
call %win32_cmd_link%

popd
