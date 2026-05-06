#pragma once

// Déclarations anticipées pour éviter les dépendances circulaires
struct Matrix3;
struct Matrix4;

// Nombre complexe z = a + bi
struct Complex
{
    float a; // partie réelle
    float b; // partie imaginaire

    Complex(float reel = 0.0f, float imag = 0.0f) : a(reel), b(imag) {}

    Complex operator+(const Complex& other) const; // addition terme à terme
    Complex operator*(const Complex& other) const; // multiplication (a+bi)(c+di)
    Complex operator/(const Complex& other) const; // division par multiplication par le conjugué
    Complex conjugate() const;                     // conjugué : a - bi
    float module() const;                          // module : sqrt(a²+b²)
    float argument() const;                        // argument : atan2(b, a) en radians
};

// Vecteur 3D réel
struct Vector3
{
    float x, y, z;
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    float dot(const Vector3& other) const;   // produit scalaire x*ox + y*oy + z*oz
    Vector3 cross(const Vector3& other) const; // produit vectoriel (vecteur perpendiculaire aux deux)
};

// Quaternion q = a + bi + cj + dk
// Convention : a = partie scalaire (cos(θ/2)), (b,c,d) = axe * sin(θ/2)
struct Quaternion
{
    float a, b, c, d;

    Quaternion(float _a = 1, float _b = 0, float _c = 0, float _d = 0) : a(_a), b(_b), c(_c), d(_d) {}

    Quaternion operator+(const Quaternion& other) const; // addition composante par composante
    Quaternion operator*(const Quaternion& other) const; // produit de Hamilton (non commutatif)
    Quaternion conjugate() const;                        // conjugué : a - bi - cj - dk
    float norm() const;                                  // norme : sqrt(a²+b²+c²+d²)
    Quaternion normalize() const;                        // quaternion unitaire (norme = 1)

    // Partie b) — représentation algébrique 4x4
    // Matrice L(q) telle que L(q)*p = q*p pour tout quaternion p
    Matrix4 toMatrix() const;
    // Inverse : lit la première colonne de L(q) pour retrouver (a,b,c,d)
    static Quaternion fromMatrix(const Matrix4& M);

    // Partie c) — quaternion de rotation <-> matrice de rotation 3x3
    // Formule de Rodrigues : R = I + 2a*(bcd×) + 2*(bcd×)²
    Matrix3 toRotationMatrix() const;
    // Inverse : méthode de Shepperd (choisit le plus grand denominateur pour stabilité numérique)
    static Quaternion fromRotationMatrix(const Matrix3& R);
};

// Vecteur 4D homogène
struct Vector4
{
    float x, y, z, w;
    Vector4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) : x(_x), y(_y), z(_z), w(_w) {}

    float dot(const Vector4& other) const; // produit scalaire 4D
};

// Matrice 3x3, stockée ligne par ligne (row-major) : m[i*3+j]
struct Matrix3 {
    float m[9];

    Matrix3() {
        for (int i = 0; i < 9; ++i) m[i] = 0.0f; // initialisation à zéro
    }

    Matrix3 operator*(const Matrix3& other) const; // produit matriciel 3x3
    Matrix3 operator+(const Matrix3& other) const; // addition terme à terme
};

// Matrice 4x4, stockée ligne par ligne (row-major) : m[i*4+j]
struct Matrix4 {
    float m[16];

    Matrix4() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f; // initialisation à zéro
    }

    Matrix4 operator*(const Matrix4& other) const; // produit matriciel 4x4
    Matrix4 operator+(const Matrix4& other) const; // addition terme à terme
};

// Retourne vrai si A et B sont égales à eps près (comparaison flottante robuste)
bool matricesEgales(const Matrix4& A, const Matrix4& B, float eps = 1e-5f);
// Vérifie que L(q1+q2)==L(q1)+L(q2) et L(q1*q2)==L(q1)*L(q2) (compatibilité algébrique)
bool verifierCompatibilite(const Quaternion& q1, const Quaternion& q2);

// Partie d) — rotation d'un vecteur 3D
// Applique la rotation q via la formule quaternionique : v' = q * (0,v) * q*
Vector3 rotateByQuaternion(const Vector3& v, const Quaternion& q);
// Applique la rotation via une matrice 3x3 : v' = R * v
Vector3 rotateByMatrix(const Vector3& v, const Matrix3& R);

// Construit le quaternion de rotation q = cos(θ/2) + sin(θ/2)*(ax,ay,az)
// L'axe (ax,ay,az) est normalisé automatiquement
Quaternion makeRotation(float ax, float ay, float az, float angle);

// Partie e) — rotation décentrée autour d'un pivot
// Equivalent à : translater(-pivot) → tourner(q) → translater(+pivot)
Vector3 rotateAround(const Vector3& v, const Vector3& pivot, const Quaternion& q);

// Partie f) — transformations affines
// Mise à l'échelle non uniforme : v' = (sx*x, sy*y, sz*z)
Vector3 applyScale(const Vector3& v, float sx, float sy, float sz);
// Translation : v' = v + (tx, ty, tz)
Vector3 applyTranslate(const Vector3& v, float tx, float ty, float tz);
// Cisaillement : x'=x+hxy*y+hxz*z, y'=hyx*x+y+hyz*z, z'=hzx*x+hzy*y+z
Vector3 applyShear(const Vector3& v, float hxy, float hxz,
                                     float hyx, float hyz,
                                     float hzx, float hzy);
