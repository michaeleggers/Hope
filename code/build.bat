@echo off

mkdir ..\build
pushd ..\build

REM HH own build
REM cl -Zi /FC ..\code\win32_handmade.cpp user32.lib gdi32.lib dsound.lib

REM OpenGL renderer
cl /Zi /FC ..\code\oglrenderer.cpp user32.lib gdi32.lib opengl32.lib dwmapi.lib glu32.lib

REM test legacy example Nehe
REM cl /Zi /FC ..\code\ogltest.cpp user32.lib gdi32.lib opengl32.lib dwmapi.lib glu32.lib

REM My SDL setup debug build
REM cl -Zi /Od /MDd /I ..\code\sdl_includes ..\code\sdltest.cpp ..\code\importlibs\SDL2main.lib ..\code\importlibs\SDL2.lib ..\code\importlibs\SDL2test.lib /link /SUBSYSTEM:CONSOLE

REM casey's build from the stream
REM cl -Zi /FC ..\code\win32_handmade_casey.cpp user32.lib gdi32.lib

REM this tests the use of keyword 'extern'
REM cl -Zi /FC ..\code\mainprog.cpp ..\code\somefunctions.cpp

popd
