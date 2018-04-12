@echo off

mkdir ..\build
pushd ..\build

REM OpenGL renderer
cl /Zi /FC ..\code\oglrenderer.cpp user32.lib gdi32.lib opengl32.lib dwmapi.lib glu32.lib


popd
