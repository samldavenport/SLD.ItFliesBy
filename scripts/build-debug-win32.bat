@echo off
@echo off

pushd ..

@set dir_bin=    build\debug\bin
@set dir_lib=    build\debug\lib
@set dir_obj=    build\debug\obj

@set cl_in=      src\win32\win32-main.cpp
@set cl_out=     /Fo:build\debug\obj\ItFliesBy.obj
@set cl_include= /Iinclude /Isrc\win32 /ISLD.Core/Include
@set cl_flags=   /nologo /c /MD /Z7 /EHs- /std:c++17 /Od /D_HAS_EXCEPTIONS=0

@set link_in=    ItFliesBy.obj user32.lib kernel32.lib
@set link_out=   /OUT:build\debug\bin\ItFliesBy.exe
@set link_path=  /LIBPATH:build\debug\obj /LIBPATH:build\debug\lib
@set link_flags= /nologo /SUBSYSTEM:WINDOWS /DEBUG

@set cmd_cl=     cl.exe   %cl_in%      %cl_out%    %cl_include% %cl_flags%
@set cmd_link=   link.exe %link_flags% %link_path% %link_in%    %link_out%

IF NOT EXIST %dir_bin% mkdir %dir_bin%
IF NOT EXIST %dir_lib% mkdir %dir_lib%
IF NOT EXIST %dir_obj% mkdir %dir_obj%

call %cmd_cl%
call %cmd_link%

popd
