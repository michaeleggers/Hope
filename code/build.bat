@echo off

mkdir ..\build
pushd ..\build

REM render device implementation for windows
cl /Zi /FC /MTd ..\code\win32_opengl.cpp ..\code\helper.cpp opengl32.lib glu32.lib gdi32.lib user32.lib /link /DLL /DEF:..\code\renderer_gl.def /OUT:win32_opengl.dll

REM kernel
cl /Zi /FC ..\code\win32_platform.cpp ..\code\helper.cpp user32.lib gdi32.lib dwmapi.lib

REM cl /Zi /FC ..\code\foo.cpp

popd
