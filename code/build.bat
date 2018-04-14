@echo off

mkdir ..\build
pushd ..\build

REM OpenGL renderer
cl /Zi /FC ..\code\win32_platform.cpp user32.lib gdi32.lib opengl32.lib dwmapi.lib glu32.lib

REM cl /Zi /FC ..\code\foo.cpp

popd
