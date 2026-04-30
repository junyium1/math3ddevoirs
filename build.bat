@echo off
setlocal

set INCLUDES=-I include
set STD=-std=c++17
set WARN=-Wall -Wextra

if "%1"=="debug" (
    set OPT=-O0 -g
    echo [build] mode: debug
) else (
    set OPT=-O2
    echo [build] mode: release  ^(pass "debug" for -g^)
)

rem --- ImGui/GLFW active seulement si setup.bat a ete lance ---
set IMGUI=
set LIBS=
set DEFINES=
if exist vendor\imgui\imgui.cpp (
    set IMGUI=vendor\imgui\imgui.cpp vendor\imgui\imgui_draw.cpp vendor\imgui\imgui_tables.cpp vendor\imgui\imgui_widgets.cpp vendor\imgui\backends\imgui_impl_glfw.cpp vendor\imgui\backends\imgui_impl_opengl3.cpp
    set INCLUDES=%INCLUDES% -I vendor\imgui -I vendor\imgui\backends
)
if exist libs\glfw3.lib (
    set LIBS=libs\glfw3.lib -lopengl32 -lgdi32 -luser32 -lshell32
)
if exist libs\glew32s.lib (
    set DEFINES=-DGLEW_STATIC
    set LIBS=%LIBS% libs\glew32s.lib
)

clang++ %STD% %OPT% %WARN% %DEFINES% %INCLUDES% src\main.cpp include\azizmath.cpp %IMGUI% %LIBS% -o build\main.exe

if errorlevel 1 (
    echo [build] FAILED
    exit /b 1
)
echo [build] OK -- build\main.exe
