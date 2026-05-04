$clang = if     (Test-Path "..\tools\clang\bin\clang++.exe")            { "..\tools\clang\bin\clang++.exe" }
         elseif (Test-Path "C:\Program Files\LLVM\bin\clang++.exe")   { "C:\Program Files\LLVM\bin\clang++.exe" }
         else   { "clang++" }

if ($args[0] -eq "debug") {
    Write-Host "[build] mode: debug"
    $opt = @("-O0", "-g")
} else {
    Write-Host "[build] mode: release"
    $opt = @("-O2")
}

# ── Chemins MSVC + Windows SDK détectés automatiquement ───────────────────────
$msvcRoot = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"
$msvcVer  = (Get-ChildItem $msvcRoot | Sort-Object Name -Descending | Select-Object -First 1).Name
$msvcInc  = "$msvcRoot\$msvcVer\include"
$msvcLib  = "$msvcRoot\$msvcVer\lib\x64"

$sdkIncRoot = "C:\Program Files (x86)\Windows Kits\10\Include"
$sdkVer     = (Get-ChildItem $sdkIncRoot | Sort-Object Name -Descending | Select-Object -First 1).Name
$ucrtInc    = "$sdkIncRoot\$sdkVer\ucrt"
$umInc      = "$sdkIncRoot\$sdkVer\um"
$sharedInc  = "$sdkIncRoot\$sdkVer\shared"

$sdkLibRoot = "C:\Program Files (x86)\Windows Kits\10\Lib"
$sdkLibVer  = (Get-ChildItem $sdkLibRoot | Sort-Object Name -Descending | Select-Object -First 1).Name
$ucrtLib    = "$sdkLibRoot\$sdkLibVer\ucrt\x64"
$umLib      = "$sdkLibRoot\$sdkLibVer\um\x64"

# ── Compilation ───────────────────────────────────────────────────────────────
$includes = @(
    "-I",       "..\include",
    "-I",       "..\vendor\imgui",
    "-I",       "..\vendor\imgui\backends",
    "-isystem", $msvcInc,
    "-isystem", $ucrtInc,
    "-isystem", $umInc,
    "-isystem", $sharedInc
)

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

$libs = @(
    "..\libs\glfw3.lib",
    "..\libs\glew32s.lib",
    "-L", $msvcLib,
    "-L", $ucrtLib,
    "-L", $umLib,
    "-lopengl32", "-lgdi32", "-luser32", "-lshell32", "-lwinmm"
)

$defines = @("-DGLEW_STATIC", "-DGLEW_NO_GLU", "-D_USE_MATH_DEFINES", "-DIMGUI_DISABLE_SSE")


& $clang -std=c++17 @opt @defines @includes @sources @libs -o build\solarsystem.exe

if ($LASTEXITCODE -ne 0) { Write-Host "[build] FAILED"; exit 1 }
Write-Host "[build] OK -- build\solarsystem.exe"
