# math3ddevoirs

Bibliotheque mathematique 3D **azizmath** et deux applications C++/OpenGL :
un visualiseur de rotations 3D interactif et un systeme solaire avec textures.

**[Documentation complete d azizmath](AZIZMATH.md)**

---

## Applications

| App | Description | Executable |
|---|---|---|
| Visualiseur de rotations | Quaternions, matrices, transformations affines en temps reel (ImGui) | `build/main.exe` |
| Systeme solaire | Planetes texturees, orbites, anneaux, eclairage Blinn-Phong (OpenGL 3.3) | `scene graphique/build/solarsystem.exe` |

---

## Prerequis

- Windows 10/11
- Visual Studio 2022 ou VS 2026 Insiders avec le composant **Developpement Desktop C++**

---

## Installation et compilation

**1. Telecharger les dependances**

```powershell
.\setup.ps1
```

Telecharge automatiquement ImGui 1.91.6, GLFW 3.4, GLEW 2.2.0 et stb_image.h.

**2. Visualiseur de rotations**

```powershell
.\build.ps1
```

**3. Systeme solaire**

```powershell
cd "scene graphique"
.\build.ps1
```

L executable se lance automatiquement apres compilation.

---

## Structure

```
math3ddevoirs/
include/azizmath.h + azizmath.cpp   # Bibliotheque mathematique
src/main.cpp                         # App 1 - Visualiseur de rotations
scene graphique/src/main.cpp         # App 2 - Systeme solaire
scene graphique/shaders/             # Vertex + fragment shaders
scene graphique/textures/            # Textures planetaires
vendor/imgui/                        # Dear ImGui
libs/                                # glfw3.lib, glew32s.lib
setup.ps1 / build.ps1
AZIZMATH.md                          # Documentation de la bibliotheque
```

---

## Bibliotheque azizmath

Ecrite entierement de zero, sans dependances.

| Type | Description |
|---|---|
| Complex | Arithmetique complexe (add, mul, div, conjugue, module, argument) |
| Vector3 | Vecteur 3D (produit scalaire, produit vectoriel) |
| Vector4 | Vecteur 4D homogene |
| Matrix3 / Matrix4 | Matrices row-major avec addition et produit |
| Quaternion | Produit de Hamilton, conjugue, norme, normalisation, conversions matrice/quaternion |
| Helpers | makeRotation, rotateByQuaternion, rotateByMatrix, rotateAround, applyScale, applyTranslate, applyShear |

Voir **[AZIZMATH.md](AZIZMATH.md)** pour la documentation complete avec formules et exemples de code.