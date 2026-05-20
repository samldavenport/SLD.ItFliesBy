@echo off
@echo off

pushd ..

::--------------------------------------------------------------------
:: BUILD DIRECTORIES
::--------------------------------------------------------------------

@set dir_bin=    build\debug\bin
@set dir_lib=    build\debug\lib
@set dir_obj=    build\debug\obj

IF NOT EXIST %dir_bin% mkdir %dir_bin%
IF NOT EXIST %dir_lib% mkdir %dir_lib%
IF NOT EXIST %dir_obj% mkdir %dir_obj%

::--------------------------------------------------------------------
:: ENGINE 
::--------------------------------------------------------------------

@set eng_cl_in=      src\engine\eng.cpp
@set eng_cl_out=     /Fo:build\debug\obj\ItFliesBy.Engine.obj
@set eng_cl_include= /Iinclude /Isrc\engine /ISLD.Core/Include
@set eng_cl_flags=   /nologo /c /MD /LD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0 

@set eng_link_in=    ItFliesBy.Engine.obj 
@set eng_link_out=   /OUT:build\debug\bin\ItFliesBy.Engine.dll /IMPLIB:build\debug\lib\ItFliesBy.Engine.lib
@set eng_link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib
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
@set win32_cl_include= /Iinclude /Isrc\win32 /ISLD.Core/Include
@set win32_cl_flags=   /nologo /c /MD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0

@set win32_link_in=    ItFliesBy.obj ItFliesBy.Engine.lib user32.lib kernel32.lib 
@set win32_link_out=   /OUT:build\debug\bin\ItFliesBy.exe
@set win32_link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib
@set win32_link_flags= /nologo /SUBSYSTEM:WINDOWS /DEBUG

@set win32_cmd_cl=     cl.exe   %win32_cl_in%      %win32_cl_out%    %win32_cl_include% %win32_cl_flags%
@set win32_cmd_link=   link.exe %win32_link_flags% %win32_link_path% %win32_link_in%    %win32_link_out%

call %win32_cmd_cl%
call %win32_cmd_link%

popd
