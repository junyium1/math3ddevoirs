$ErrorActionPreference = 'Stop'

# ── Mode debug / release ──────────────────────────────────────────────────────
if ($args[0] -eq "debug") {
    Write-Host "[build] mode: debug"
    $opt = @("/Od", "/Zi")
} else {
    Write-Host "[build] mode: release"
    $opt = @("/O2")
}

# ── Détection MSVC ────────────────────────────────────────────────────────────
$msvcRoot = $null
foreach ($vsBase in @(
    "C:\Program Files\Microsoft Visual Studio\18\Insiders",
    "C:\Program Files\Microsoft Visual Studio\2022\Community",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
)) {
    $candidate = "$vsBase\VC\Tools\MSVC"
    if (Test-Path $candidate) { $msvcRoot = $candidate; break }
}
if (-not $msvcRoot) { Write-Host "[build] ERREUR: MSVC introuvable"; exit 1 }

$msvcVer = (Get-ChildItem $msvcRoot | Sort-Object Name -Descending | Select-Object -First 1).Name
$msvcInc = "$msvcRoot\$msvcVer\include"
$msvcLib = "$msvcRoot\$msvcVer\lib\x64"
$cl      = "$msvcRoot\$msvcVer\bin\Hostx64\x64\cl.exe"

# ── Détection Windows SDK ─────────────────────────────────────────────────────
$sdkIncRoot = "C:\Program Files (x86)\Windows Kits\10\Include"
$sdkVer     = (Get-ChildItem $sdkIncRoot | Sort-Object Name -Descending | Select-Object -First 1).Name
$ucrtInc    = "$sdkIncRoot\$sdkVer\ucrt"
$umInc      = "$sdkIncRoot\$sdkVer\um"
$sharedInc  = "$sdkIncRoot\$sdkVer\shared"

$sdkLibRoot = "C:\Program Files (x86)\Windows Kits\10\Lib"
$sdkLibVer  = (Get-ChildItem $sdkLibRoot | Sort-Object Name -Descending | Select-Object -First 1).Name
$ucrtLib    = "$sdkLibRoot\$sdkLibVer\ucrt\x64"
$umLib      = "$sdkLibRoot\$sdkLibVer\um\x64"

# ── Dossier build ─────────────────────────────────────────────────────────────
New-Item -ItemType Directory -Force -Path "build" | Out-Null

# ── Sources ───────────────────────────────────────────────────────────────────
$sources = @(
    "src\main.cpp",
    "..\include\azizmath.cpp",
    "..\vendor\imgui\imgui.cpp",
    "..\vendor\imgui\imgui_draw.cpp",
    "..\vendor\imgui\imgui_tables.cpp",
    "..\vendor\imgui\imgui_widgets.cpp",
    "..\vendor\imgui\backends\imgui_impl_glfw.cpp",
    "..\vendor\imgui\backends\imgui_impl_opengl3.cpp"
)

# ── Compilation ───────────────────────────────────────────────────────────────
$includes = @(
    "/I", "..\include",
    "/I", "..\vendor\imgui",
    "/I", "..\vendor\imgui\backends",
    "/I", $msvcInc,
    "/I", $ucrtInc,
    "/I", $umInc,
    "/I", $sharedInc
)

$libs = @(
    "..\libs\glfw3.lib",
    "..\libs\glew32s.lib",
    "opengl32.lib",
    "gdi32.lib",
    "user32.lib",
    "shell32.lib",
    "winmm.lib",
    "/LIBPATH:$msvcLib",
    "/LIBPATH:$ucrtLib",
    "/LIBPATH:$umLib"
)

$defines = @("/DGLEW_STATIC", "/DGLEW_NO_GLU", "/D_USE_MATH_DEFINES", "/DIMGUI_DISABLE_SSE", "/D_CRT_SECURE_NO_WARNINGS")

Write-Host "[build] Compilation avec cl.exe (MSVC $msvcVer)..."
& $cl /std:c++17 @opt /MD /W3 /EHsc @defines @includes @sources /Fe:build\solarsystem.exe /Fo:build\ /link @libs

if ($LASTEXITCODE -ne 0) { Write-Host "[build] FAILED"; exit 1 }
Write-Host "[build] OK -- build\solarsystem.exe"
Write-Host "[build] Lancement de build\solarsystem.exe ..."
Start-Process -FilePath ".\build\solarsystem.exe" -WorkingDirectory (Get-Location).Path
