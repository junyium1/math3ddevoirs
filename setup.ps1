$ProgressPreference = 'SilentlyContinue'
$ErrorActionPreference = 'Stop'

# ── Dossiers ──────────────────────────────────────────────────────────────────
Write-Host "[setup] Creating directories..."
foreach ($d in @('src','vendor','include','libs','build','tools','scene graphique\build')) {
    New-Item -ItemType Directory -Force -Path $d | Out-Null
}

# ── Clang (local, portable) ───────────────────────────────────────────────────
if (Test-Path 'tools\clang\bin\clang++.exe') {
    Write-Host "[setup] Clang deja present dans tools\clang\"
} else {
    $llvmVer = "19.1.7"
    $llvmTar = "clang+llvm-$llvmVer-x86_64-pc-windows-msvc.tar.xz"
    Write-Host "[setup] Downloading LLVM $llvmVer (~300MB)..."
    Start-BitsTransfer -Source "https://github.com/llvm/llvm-project/releases/download/llvmorg-$llvmVer/$llvmTar" -Destination "tools\$llvmTar"
    Write-Host "[setup] Extracting LLVM..."
    New-Item -ItemType Directory -Force -Path "tools\clang" | Out-Null
    tar -xf "tools\$llvmTar" -C "tools\clang" --strip-components=1
    Remove-Item "tools\$llvmTar"
    Write-Host "[setup] Clang installe dans tools\clang\"
}

# ── ImGui ─────────────────────────────────────────────────────────────────────
Write-Host "[setup] Downloading ImGui 1.91.6..."
Invoke-WebRequest 'https://github.com/ocornut/imgui/archive/refs/tags/v1.91.6.zip' -OutFile 'imgui.zip'
Remove-Item -Recurse -Force 'imgui-1.91.6' -ErrorAction SilentlyContinue
Expand-Archive 'imgui.zip' -DestinationPath '.' -Force
Remove-Item -Recurse -Force 'vendor\imgui' -ErrorAction SilentlyContinue
Move-Item 'imgui-1.91.6' 'vendor\imgui'
Remove-Item 'imgui.zip'

# ── GLFW ──────────────────────────────────────────────────────────────────────
Write-Host "[setup] Downloading GLFW 3.4..."
Invoke-WebRequest 'https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip' -OutFile 'glfw.zip'
Remove-Item -Recurse -Force 'glfw-3.4.bin.WIN64' -ErrorAction SilentlyContinue
Expand-Archive 'glfw.zip' -DestinationPath '.' -Force
Copy-Item -Recurse -Force 'glfw-3.4.bin.WIN64\include\*' 'include\'
Copy-Item -Force 'glfw-3.4.bin.WIN64\lib-vc2022\glfw3.lib' 'libs\'
Remove-Item -Recurse -Force 'glfw-3.4.bin.WIN64'
Remove-Item 'glfw.zip'

# ── GLEW ──────────────────────────────────────────────────────────────────────
Write-Host "[setup] Downloading GLEW 2.2.0..."
Invoke-WebRequest 'https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip' -OutFile 'glew.zip'
Remove-Item -Recurse -Force 'glew-2.2.0' -ErrorAction SilentlyContinue
Expand-Archive 'glew.zip' -DestinationPath '.' -Force
Copy-Item -Recurse -Force 'glew-2.2.0\include\*' 'include\'
Copy-Item -Force 'glew-2.2.0\lib\Release\x64\glew32s.lib' 'libs\'
Remove-Item -Recurse -Force 'glew-2.2.0'
Remove-Item 'glew.zip'

# ── stb_image ─────────────────────────────────────────────────────────────────
Write-Host "[setup] Downloading stb_image.h..."
Invoke-WebRequest 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile 'include\stb_image.h'

# ── Done ──────────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "[setup] Done."
Write-Host "  tools\clang\       - Clang/LLVM portable"
Write-Host "  vendor\imgui\      - ImGui source"
Write-Host "  include\GLFW\      - GLFW headers"
Write-Host "  include\GL\        - GLEW headers"
Write-Host "  include\stb_image.h"
Write-Host "  libs\glfw3.lib"
Write-Host "  libs\glew32s.lib"
Write-Host ""
Write-Host "Lance build.ps1 pour compiler."
