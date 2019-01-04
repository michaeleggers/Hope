@echo off

mkdir ..\build
pushd ..\build

REM render device implementation for windows
cl /Zi /FC /MTd ..\code\ogl_render.cpp ..\code\win32_common_os.cpp ..\code\helper.cpp opengl32.lib glu32.lib gdi32.lib user32.lib /link /DLL /DEF:..\code\renderer_gl.def /OUT:win32_opengl.dll

REM kernel
cl /Zi /FC ..\code\win32_platform.cpp ..\code\helper.cpp user32.lib gdi32.lib dwmapi.lib xinput.lib

REM cl /Zi /FC ..\code\foo.cpp

popd
