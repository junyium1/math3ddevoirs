@echo off
setlocal

echo [setup] Creating directories...
mkdir src     2>nul
mkdir vendor  2>nul
mkdir include 2>nul
mkdir libs    2>nul
mkdir build   2>nul

echo [setup] Downloading ImGui 1.91.6...
powershell -NoProfile -Command "Invoke-WebRequest 'https://github.com/ocornut/imgui/archive/refs/tags/v1.91.6.zip' -OutFile 'imgui.zip' -UseBasicParsing"
rmdir /s /q imgui-1.91.6 2>nul
powershell -NoProfile -Command "Add-Type -A System.IO.Compression.FileSystem; [IO.Compression.ZipFile]::ExtractToDirectory((Resolve-Path 'imgui.zip').Path, (Get-Location).Path)"
rmdir /s /q vendor\imgui 2>nul
move imgui-1.91.6 vendor\imgui
del imgui.zip

echo [setup] Downloading GLFW 3.4 (Windows 64-bit)...
powershell -NoProfile -Command "Invoke-WebRequest 'https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip' -OutFile 'glfw.zip' -UseBasicParsing"
rmdir /s /q glfw-3.4.bin.WIN64 2>nul
powershell -NoProfile -Command "Add-Type -A System.IO.Compression.FileSystem; [IO.Compression.ZipFile]::ExtractToDirectory((Resolve-Path 'glfw.zip').Path, (Get-Location).Path)"
xcopy /e /q /i glfw-3.4.bin.WIN64\include\* include\ >nul
copy /y glfw-3.4.bin.WIN64\lib-vc2022\glfw3.lib libs\ >nul
copy /y glfw-3.4.bin.WIN64\lib-vc2022\glfw3.dll build\ >nul
rmdir /s /q glfw-3.4.bin.WIN64
del glfw.zip

echo [setup] Downloading GLEW 2.2.0 (Windows 64-bit)...
powershell -NoProfile -Command "Invoke-WebRequest 'https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip' -OutFile 'glew.zip' -UseBasicParsing"
rmdir /s /q glew-2.2.0 2>nul
powershell -NoProfile -Command "Add-Type -A System.IO.Compression.FileSystem; [IO.Compression.ZipFile]::ExtractToDirectory((Resolve-Path 'glew.zip').Path, (Get-Location).Path)"
xcopy /e /q /i glew-2.2.0\include\* include\ >nul
copy /y "glew-2.2.0\lib\Release\x64\glew32s.lib" libs\ >nul
rmdir /s /q glew-2.2.0
del glew.zip

echo [setup] Downloading stb_image.h...
powershell -NoProfile -Command "Invoke-WebRequest 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile 'include\stb_image.h' -UseBasicParsing"

echo [setup] Done.
echo         include\       — math headers + GLFW + GLEW + stb_image.h
echo         include\GL\    — glew.h
echo         vendor\imgui\  — ImGui source
echo         libs\          — glfw3.lib  glew32s.lib
echo         build\         — compiled output + glfw3.dll
echo.
echo         Run build.bat to compile.
