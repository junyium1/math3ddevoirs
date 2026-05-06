# Documentation — Bibliothèque azizmath

> Référence technique complète : API, formules, implémentation et application dans le code.

---

## Table des matières

1. [Vue d'ensemble](#vue-densemble)
2. [Complex](#complex)
3. [Vector3](#vector3)
4. [Vector4](#vector4)
5. [Matrix3 et Matrix4](#matrix3-et-matrix4)
6. [Quaternion](#quaternion)
7. [Fonctions globales](#fonctions-globales)
8. [Application 1 — Visualiseur de rotations 3D](#application-1--visualiseur-de-rotations-3d)
9. [Application 2 — Système solaire OpenGL](#application-2--système-solaire-opengl)
10. [Fondements mathématiques](#fondements-mathématiques)

---

## Vue d'ensemble

Fichiers : `include/azizmath.h` + `include/azizmath.cpp`

Bibliothèque mathématique 3D écrite entièrement de zéro (sans dépendances),
couvrant les nombres complexes, les vecteurs, les matrices et les quaternions
avec toutes les opérations nécessaires pour les rotations et transformations 3D.

---

## Complex

```cpp
struct Complex {
	float a; // partie réelle
	float b; // partie imaginaire
};
```

Un nombre complexe `z = a + bi`.

| Méthode | Formule | Description |
|---|---|---|
| `operator+` | `(a+c) + (b+d)i` | Addition terme à terme |
| `operator*` | `(ac−bd) + (bc+ad)i` | Multiplication avec les règles i²=−1 |
| `operator/` | `(a+bi)(c−di) / (c²+d²)` | Division par le conjugué du dénominateur |
| `conjugate()` | `a − bi` | Inverse le signe imaginaire |
| `module()` | `√(a²+b²)` | Distance à l'origine dans le plan complexe |
| `argument()` | `atan2(b, a)` | Angle polaire en radians ∈ [−π, π] |

**Division — détail :**
```
(a+bi)   (a+bi)(c−di)   (ac+bd) + (bc−ad)i
──────  = ──────────── = ─────────────────────
(c+di)      c²+d²              c²+d²
```

---

## Vector3

```cpp
struct Vector3 {
	float x, y, z;
};
```

Vecteur 3D flottant.

| Méthode | Formule | Description |
|---|---|---|
| `dot(v)` | `x·vx + y·vy + z·vz` | Produit scalaire (0 = perpendiculaires, 1 = parallèles) |
| `cross(v)` | `(yz'−zy', zx'−xz', xy'−yx')` | Produit vectoriel (vecteur perpendiculaire aux deux) |

**Produit vectoriel — détail (règle de la main droite) :**
```
	   | i   j   k  |
u × v = | ux  uy  uz |  =  (uy·vz − uz·vy, uz·vx − ux·vz, ux·vy − uy·vx)
	   | vx  vy  vz |
```

---

## Vector4

```cpp
struct Vector4 {
	float x, y, z, w;
};
```

Vecteur 4D homogène (utilisé pour les calculs matriciels 4×4).

| Méthode | Formule | Description |
|---|---|---|
| `dot(v)` | `x·vx + y·vy + z·vz + w·vw` | Produit scalaire 4D |

---

## Matrix3 et Matrix4

Stockées **ligne par ligne** (row-major) :
- `Matrix3::m[i*3+j]` = élément à la ligne `i`, colonne `j`
- `Matrix4::m[i*4+j]` = élément à la ligne `i`, colonne `j`

```cpp
struct Matrix3 { float m[9];  };   // initialisée à zéro
struct Matrix4 { float m[16]; };   // initialisée à zéro
```

| Méthode | Description |
|---|---|
| `operator+` | Addition terme à terme |
| `operator*` | Produit matriciel O(n³) : `result[i][j] = Σ_k A[i][k] * B[k][j]` |

**Produit matriciel 3×3 :**
```
(A·B)[i][j] = A[i][0]·B[0][j] + A[i][1]·B[1][j] + A[i][2]·B[2][j]
```

---

## Quaternion

```cpp
struct Quaternion {
	float a, b, c, d;
	// q = a + bi + cj + dk
	// Convention rotation : a = cos(θ/2),  (b,c,d) = axe_normalisé × sin(θ/2)
};
```

> Un quaternion unitaire (‖q‖=1) représente une rotation 3D
> sans souffrir du blocage de cardan (_gimbal lock_).

### Opérations de base

| Méthode | Formule | Description |
|---|---|---|
| `operator+` | `(a+a', b+b', c+c', d+d')` | Addition composante par composante |
| `conjugate()` | `a − bi − cj − dk` | Inverse la partie vectorielle (= rotation inverse) |
| `norm()` | `√(a²+b²+c²+d²)` | Norme euclidienne (doit valoir 1 pour une rotation) |
| `normalize()` | `q / ‖q‖` | Rend le quaternion unitaire |

### Produit de Hamilton `operator*`

Le produit de quaternions est **non commutatif** (`q1*q2 ≠ q2*q1`).
Il encode la **composition de rotations**.

Avec `i²=j²=k²=ijk=−1` et les règles `ij=k, jk=i, ki=j` :

```
(a₁+b₁i+c₁j+d₁k)(a₂+b₂i+c₂j+d₂k) =

  a_résultat = a₁a₂ − b₁b₂ − c₁c₂ − d₁d₂
  b_résultat = a₁b₂ + b₁a₂ + c₁d₂ − d₁c₂
  c_résultat = a₁c₂ − b₁d₂ + c₁a₂ + d₁b₂
  d_résultat = a₁d₂ + b₁c₂ − c₁b₂ + d₁a₂
```

> **Application dans le code :**
> Dans `src/main.cpp`, la composition `qC = q1 * q2` applique d'abord q2, puis q1 :
> ```cpp
> Quaternion qC = q1 * q2; // q2 en premier, puis q1
> ```

### `toMatrix()` — Représentation algébrique 4×4

Construit la matrice de **multiplication à gauche** L(q) telle que `L(q)·p = q·p`
pour tout quaternion p représenté comme vecteur colonne `(a,b,c,d)ᵀ`.

```
		| a  -b  -c  -d |
L(q) =  | b   a  -d   c |
		| c   d   a  -b |
		| d  -c   b   a |
```

Cette structure antisymétrique découle directement des règles du produit de Hamilton.
**Vérification :** `L(q1·q2) = L(q1)·L(q2)` — testé par `verifierCompatibilite()`.

### `fromMatrix()` — Retour depuis L(q)

La première colonne de L(q) contient directement `(a, b, c, d)` :
```cpp
Quaternion::fromMatrix(M)  →  (M[0][0], M[1][0], M[2][0], M[3][0])
							=  (a, b, c, d)
```

### `toRotationMatrix()` — Matrice de rotation 3×3

Convertit un quaternion unitaire en matrice de rotation 3×3 par la formule de Rodrigues développée :

```
	   | 1−2c²−2d²    2bc−2da     2bd+2ca  |
R(q) = | 2bc+2da      1−2b²−2d²   2cd−2ba  |
	   | 2bd−2ca      2cd+2ba     1−2b²−2c²|
```

> **Preuve :** En développant `v' = q·(0,v)·q*` avec q = (a,b,c,d), on obtient
> exactement ces coefficients après simplification avec ‖q‖=1.

### `fromRotationMatrix()` — Méthode de Shepperd

Inverse la conversion : matrice de rotation 3×3 → quaternion.
Utilise la **méthode de Shepperd** qui choisit le plus grand des quatre dénominateurs
pour garantir la stabilité numérique (évite la division par zéro) :

```
t₀ = 1 + R₀₀ + R₁₁ + R₂₂  →  4a²
t₁ = 1 + R₀₀ − R₁₁ − R₂₂  →  4b²
t₂ = 1 − R₀₀ + R₁₁ − R₂₂  →  4c²
t₃ = 1 − R₀₀ − R₁₁ + R₂₂  →  4d²
```

On choisit le `tᵢ` le plus grand, on extrait `qᵢ = √tᵢ / 2`,
puis les autres composantes par les formules antisymétriques de R.

---

## Fonctions globales

### `makeRotation(ax, ay, az, angle)`

Construit le quaternion de rotation autour de l'axe `(ax,ay,az)` d'un angle `θ` (radians).

```
1. Normalise l'axe : û = (ax,ay,az) / ‖(ax,ay,az)‖
2. q = cos(θ/2) + sin(θ/2)·(ûx·i + ûy·j + ûz·k)
```

Retourne l'identité `(1,0,0,0)` si l'axe est nul.

> **Application dans le code (système solaire) :**
> ```cpp
> Quaternion q = makeRotation(sinf(0.0f), cosf(0.0f), 0, t * 1.00f);
> Vector3 pos = rotateByQuaternion(Vector3(7.5f, 0, 0), q);
> ```

### `rotateByQuaternion(v, q)`

Applique la rotation `q` au vecteur `v` via la **sandwich formula** :

```
p = (0, vx, vy, vz)   ← quaternion pur
v' = q · p · q*        ← sandwich
résultat = (r.b, r.c, r.d)
```

Coût : **28 multiplications + 21 additions**.

> ```cpp
> Vector3 rQ = rotateByQuaternion(v, q1);
> ```

### `rotateByMatrix(v, R)`

Applique la rotation via le produit matrice-vecteur `v' = R·v`.

Coût : **9 multiplications + 6 additions**.

> Les deux méthodes donnent le même résultat.
> Dans `src/main.cpp`, l'erreur est calculée et affichée en temps réel :
> ```cpp
> float err = sqrtf(dx*dx + dy*dy + dz*dz); // doit être < 1e-5
> ```

### `rotateAround(v, pivot, q)`

Rotation décentrée autour d'un pivot arbitraire en 3 étapes :

```
1. Translater à l'origine  : local = v − pivot
2. Tourner                 : rotated = rotateByQuaternion(local, q)
3. Replacer                : résultat = rotated + pivot
```

> ```cpp
> Vector3 rDec = rotateAround(v, piv, q1);
> ```

### `applyScale(v, sx, sy, sz)`

Mise à l'échelle non uniforme : `v' = (sx·x, sy·y, sz·z)`.

### `applyTranslate(v, tx, ty, tz)`

Translation : `v' = v + (tx, ty, tz)`.

### `applyShear(v, hxy, hxz, hyx, hyz, hzx, hzy)`

Cisaillement : déforme l'espace en inclinant chaque axe selon les autres.

```
x' = x + hxy·y + hxz·z
y' = hyx·x + y + hyz·z
z' = hzx·x + hzy·y + z
```

### `matricesEgales(A, B, eps)`

Compare deux `Matrix4` composante par composante avec tolérance `eps = 1e-5`.

### `verifierCompatibilite(q1, q2)`

Vérifie deux propriétés fondamentales de la représentation matricielle L(q) :

```
L(q1 + q2) == L(q1) + L(q2)   ← linéarité additive
L(q1 * q2) == L(q1) * L(q2)   ← compatibilité multiplicative
```

Retourne `true` si les deux propriétés sont vérifiées (à `eps` près).

---

## Application 1 — Visualiseur de rotations 3D

**Fichier :** `src/main.cpp` — **Exécutable :** `build/main.exe`

Interface entièrement dans ImGui, rendu 3D via `ImDrawList` (pas d'OpenGL direct).

### Panneaux ImGui

| Section | Ce que ça fait |
|---|---|
| **Axe / Angle** | Définit q1 = makeRotation(axe, angle), animation possible |
| **Point v** | Vecteur à tourner, comparaison quaternion vs matrice |
| **Pivot** | Rotation décentrée avec rotateAround() |
| **Quaternion** | Affiche (a,b,c,d) et la norme de q1 en temps réel |
| **Résultats** | rQ (via quaternion), rM (via matrice), erreur numérique |
| **Exemple cours** | Rotation 120° autour de (1/√3, 1/√3, 1/√3), résultat attendu (c,a,b) |
| **Composition** | q2 puis q1 : qC = q1*q2, cube vert avec toutes les transformations |
| **Scaling** | applyScale() : étire le cube |
| **Translation** | applyTranslate() : déplace le cube |
| **Cisaillement** | applyShear() : déforme le cube |

### Viewport 3D

| Couleur | Transformation |
|---|---|
| 🔘 Gris foncé | Position initiale |
| 🔵 Bleu | Rotation q1 centrée (rotateByMatrix) |
| 🟠 Orange | Rotation q1 décentrée autour du pivot (rotateAround) |
| 🟢 Vert | Toutes les transformations actives (scale → rotate → shear → translate) |

**Contrôles souris :** clic gauche = orbite, clic droit = pan, scroll = zoom.

---

## Application 2 — Système solaire OpenGL

**Fichier :** `scene graphique/src/main.cpp` — **Exécutable :** `scene graphique/build/solarsystem.exe`

Système solaire 3D avec textures, éclairage Blinn-Phong et anneaux planétaires.
**Toutes les rotations passent par azizmath** — aucune matrice de rotation écrite à la main.

### Orbites planétaires

```cpp
Quaternion q = makeRotation(
	sinf(p.orbInclination),
	cosf(p.orbInclination),
	0,
	t * p.orbitSpeed * speedMult
);
Vector3 pos = rotateByQuaternion(Vector3(p.orbitRadius, 0, 0), q);
```

L'inclinaison orbitale est encodée directement dans l'axe du quaternion.

### Rotation propre (spin axial)

```cpp
Quaternion qSpin = makeRotation(sinf(p.axialTilt), cosf(p.axialTilt), 0, t * p.spinSpeed * speedMult);
Mat4 model = makeModel(qSpin.toRotationMatrix(), px, py, pz, p.size);
```

Le quaternion est converti en `Matrix3` via `toRotationMatrix()` puis injecté dans la matrice modèle OpenGL.

### Caméra orbitale

```cpp
Quaternion qCam = makeRotation(0, 1, 0, camYaw) * makeRotation(1, 0, 0, -camPitch);
Vector3 eye = applyTranslate(rotateByQuaternion(Vector3(0, 0, camDist), qCam),
							 camTarget.x, camTarget.y, camTarget.z);
```

### Position de la Lune

```cpp
Vector3 moonPos = applyTranslate(
	rotateByQuaternion(Vector3(0.7f, 0, 0), qMoonOrbit),
	planetPos[2].x, planetPos[2].y, planetPos[2].z
);
```

### Données planétaires

| Planète | Rayon orbital | Vitesse orbitale | Tilt axial | Note |
|---|---|---|---|---|
| Mercure | 3.5 | 4.15 rad/s | 0.01 rad | — |
| Vénus | 5.5 | 1.62 rad/s | 3.096 rad | Rotation rétrograde |
| Terre | 7.5 | 1.00 rad/s | 0.409 rad | Référence, avec Lune |
| Mars | 9.8 | 0.53 rad/s | 0.440 rad | — |
| Jupiter | 13.5 | 0.08 rad/s | 0.054 rad | Géante gazeuse |
| Saturne | 17.5 | 0.03 rad/s | 0.467 rad | Anneaux |
| Uranus | 21.0 | 0.01 rad/s | 1.706 rad | Tilt ≈ 98° |
| Neptune | 24.0 | 0.006 rad/s | 0.494 rad | — |

### Éclairage (shader)

```glsl
float diffuse  = max(dot(N, L), 0.0) * uLightIntensity;
float specular = pow(max(dot(R, V), 0.0), 32.0) * 0.4 * uLightIntensity;
vec3 col = texColor * (uAmbientIntensity + diffuse) + vec3(specular);
col = col / (col + vec3(1.0));  // tone mapping Reinhard
```

---

## Fondements mathématiques

### Quaternion vs matrice de rotation

| Critère | Matrice 3×3 | Quaternion |
|---|---|---|
| Mémoire | 9 floats | 4 floats |
| Rotation d'un vecteur | 9 mult + 6 add | 28 mult + 21 add |
| Composition | 27 mult | 16 mult |
| Interpolation (SLERP) | Difficile | Naturelle |
| Gimbal lock | Possible (Euler) | Impossible |
| Dérive numérique | Re-orthogonalisation | Simple renormalisation |

### Relation complexe ↔ quaternion

```
z = a + bi                →  rotation 2D (angle θ dans le plan)
q = a + bi + cj + dk      →  rotation 3D (angle θ autour d'un axe)

e^(iθ) = cos θ + i·sin θ  →  rotation 2D de θ
q = cos(θ/2) + sin(θ/2)·û →  rotation 3D de θ autour de û
```

### Convention axe-angle → quaternion

```
Axe û = (ux, uy, uz) normalisé,  angle θ en radians

q = ( cos(θ/2),  ux·sin(θ/2),  uy·sin(θ/2),  uz·sin(θ/2) )
```

### Exemple — cours exercice 2

Rotation de 120° autour de l'axe (1/√3, 1/√3, 1/√3) :

```
θ/2 = π/3  →  cos(π/3) = 0.5,  sin(π/3)·(1/√3) = 0.5

q = (0.5,  0.5,  0.5,  0.5)

Rotation de v = (1, 2, 3) :
  v' = q·(0,1,2,3)·q* = (3, 1, 2)   ← cycle (c, a, b)  ✓
```

Vérifiable interactivement dans l'app 1, section **"Exemple cours"**.
