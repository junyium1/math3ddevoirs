# Lance solarsystem.exe en s'assurant que les jonctions shaders/ et textures/ existent dans build/
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

$buildDir    = Join-Path $root "build"
$shadersJunc = Join-Path $buildDir "shaders"
$texturesJunc= Join-Path $buildDir "textures"
$shadersReal = Join-Path $root "shaders"
$texturesReal= Join-Path $root "textures"
$exe         = Join-Path $buildDir "solarsystem.exe"

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

if (-not (Test-Path $shadersJunc)) {
	cmd /c mklink /J "$shadersJunc" "$shadersReal" | Out-Null
	Write-Host "[run] jonction build\shaders creee"
}
if (-not (Test-Path $texturesJunc)) {
	cmd /c mklink /J "$texturesJunc" "$texturesReal" | Out-Null
	Write-Host "[run] jonction build\textures creee"
}

if (-not (Test-Path $exe)) {
	Write-Host "[run] ERREUR : $exe introuvable. Lance build.ps1 d'abord."
	exit 1
}

Write-Host "[run] Lancement de $exe ..."
Start-Process -FilePath $exe -WorkingDirectory $buildDir
