#include "azizmath.h"
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Complex
// ─────────────────────────────────────────────────────────────────────────────

// (a+bi) + (c+di) = (a+c) + (b+d)i
Complex Complex::operator+(const Complex& other) const
{
    return Complex(a + other.a, b + other.b);
}

// (a+bi)(c+di) = (ac-bd) + (bc+ad)i
Complex Complex::operator*(const Complex& other) const
{
    float nouveau_reel = (a * other.a) - (b * other.b);
    float nouveau_imag = (b * other.a) + (a * other.b);
    return Complex(nouveau_reel, nouveau_imag);
}

// (a+bi)/(c+di) = (a+bi)(c-di) / (c²+d²) — multiplication par le conjugué du dénominateur
Complex Complex::operator/(const Complex& other) const
{
    float diviseur = (other.a * other.a) + (other.b * other.b); // |other|²

    float nouveau_reel = ((a * other.a) + (b * other.b)) / diviseur;
    float nouveau_imag = ((b * other.a) - (a * other.b)) / diviseur;

    return Complex(nouveau_reel, nouveau_imag);
}

// Conjugué : inverse le signe de la partie imaginaire
Complex Complex::conjugate() const
{
    return Complex(a, -b);
}

// Module : distance à l'origine dans le plan complexe
float Complex::module() const
{
    return std::sqrt((a * a) + (b * b));
}

// Argument : angle polaire en radians dans [-π, π]
float Complex::argument() const
{
    return std::atan2(b, a);
}

// ─────────────────────────────────────────────────────────────────────────────
// Vector3
// ─────────────────────────────────────────────────────────────────────────────

// Produit scalaire : mesure la projection de v sur other (0 = perpendiculaires)
float Vector3::dot(const Vector3& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

// Produit vectoriel : vecteur perpendiculaire aux deux, orienté par la règle de la main droite
Vector3 Vector3::cross(const Vector3& other) const
{
    float nouveau_x = (y * other.z) - (z * other.y);
    float nouveau_y = (z * other.x) - (x * other.z);
    float nouveau_z = (x * other.y) - (y * other.x);
    return Vector3(nouveau_x, nouveau_y, nouveau_z);
}

// ─────────────────────────────────────────────────────────────────────────────
// Quaternion
// ─────────────────────────────────────────────────────────────────────────────

// Addition composante par composante (utile pour interpolation, pas pour rotation)
Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return Quaternion(a + other.a, b + other.b, c + other.c, d + other.d);
}

// Produit de Hamilton : composition de rotations (non commutatif, q1*q2 ≠ q2*q1)
// Formule : (a1+b1i+c1j+d1k)(a2+b2i+c2j+d2k) avec i²=j²=k²=ijk=-1
Quaternion Quaternion::operator*(const Quaternion& other) const
{
    // partie scalaire : produit scalaire des parties vectorielles soustrait
    float nouveau_a = (a * other.a) - (b * other.b) - (c * other.c) - (d * other.d);
    float nouveau_b = (a * other.b) + (b * other.a) + (c * other.d) - (d * other.c);
    float nouveau_c = (a * other.c) - (b * other.d) + (c * other.a) + (d * other.b);
    float nouveau_d = (a * other.d) + (b * other.c) - (c * other.b) + (d * other.a);
    return Quaternion(nouveau_a, nouveau_b, nouveau_c, nouveau_d);
}

// Conjugué : inverse l'axe de rotation (rotation inverse si q est unitaire)
Quaternion Quaternion::conjugate() const
{
    return Quaternion(a, -b, -c, -d);
}

// Norme : doit valoir 1 pour un quaternion de rotation valide
float Quaternion::norm() const
{
    return std::sqrt((a * a) + (b * b) + (c * c) + (d * d));
}

// Normalise : garantit ||q||=1, nécessaire avant toute rotation
Quaternion Quaternion::normalize() const
{
    float n = norm();
    return Quaternion(a / n, b / n, c / n, d / n);
}

// ─────────────────────────────────────────────────────────────────────────────
// Vector4
// ─────────────────────────────────────────────────────────────────────────────

// Produit scalaire 4D
float Vector4::dot(const Vector4& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

// ─────────────────────────────────────────────────────────────────────────────
// Matrix3
// ─────────────────────────────────────────────────────────────────────────────

// Addition terme à terme
Matrix3 Matrix3::operator+(const Matrix3& other) const
{
    Matrix3 result;
    for (int i = 0; i < 9; ++i) result.m[i] = m[i] + other.m[i];
    return result;
}

// Produit matriciel 3x3 classique O(n³), résultat[i][j] = somme_k A[i][k]*B[k][j]
Matrix3 Matrix3::operator*(const Matrix3& other) const
{
    Matrix3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                result.m[i*3+j] += m[i*3+k] * other.m[k*3+j];
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Matrix4
// ─────────────────────────────────────────────────────────────────────────────

// Addition terme à terme
Matrix4 Matrix4::operator+(const Matrix4& other) const
{
    Matrix4 result;
    for (int i = 0; i < 16; ++i) result.m[i] = m[i] + other.m[i];
    return result;
}

// Produit matriciel 4x4 classique
Matrix4 Matrix4::operator*(const Matrix4& other) const
{
    Matrix4 result;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                result.m[i*4+j] += m[i*4+k] * other.m[k*4+j];
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Partie b) — représentation matricielle L(q)
// ─────────────────────────────────────────────────────────────────────────────

// Construit la matrice de multiplication à gauche L(q) telle que L(q)*p = q*p
// Structure antisymétrique :
// L(q) = | a  -b  -c  -d |
//        | b   a  -d   c |
//        | c   d   a  -b |
//        | d  -c   b   a |
Matrix4 Quaternion::toMatrix() const
{
    Matrix4 M;
    M.m[0]  =  a; M.m[1]  = -b; M.m[2]  = -c; M.m[3]  = -d;
    M.m[4]  =  b; M.m[5]  =  a; M.m[6]  = -d; M.m[7]  =  c;
    M.m[8]  =  c; M.m[9]  =  d; M.m[10] =  a; M.m[11] = -b;
    M.m[12] =  d; M.m[13] = -c; M.m[14] =  b; M.m[15] =  a;
    return M;
}

// Reconstruit q depuis L(q) : la première colonne contient directement (a,b,c,d)
Quaternion Quaternion::fromMatrix(const Matrix4& M)
{
    return Quaternion(M.m[0], M.m[4], M.m[8], M.m[12]);
}

// ─────────────────────────────────────────────────────────────────────────────
// Partie c) — conversion quaternion <-> matrice de rotation
// ─────────────────────────────────────────────────────────────────────────────

// Convertit un quaternion unitaire en matrice de rotation 3x3
// Formule de Rodrigues développée : R[i][j] = (a²+b²-c²-d² selon la diagonale) + croisements
Matrix3 Quaternion::toRotationMatrix() const
{
    Quaternion q = normalize();
    float a = q.a, b = q.b, c = q.c, d = q.d;

    Matrix3 R;
    R.m[0] = 1 - 2*c*c - 2*d*d;  R.m[1] = 2*b*c - 2*d*a;      R.m[2] = 2*b*d + 2*c*a;
    R.m[3] = 2*b*c + 2*d*a;      R.m[4] = 1 - 2*b*b - 2*d*d;  R.m[5] = 2*c*d - 2*b*a;
    R.m[6] = 2*b*d - 2*c*a;      R.m[7] = 2*c*d + 2*b*a;      R.m[8] = 1 - 2*b*b - 2*c*c;
    return R;
}

// Méthode de Shepperd : choisit la plus grande des quatre valeurs |a|,|b|,|c|,|d|
// pour éviter la division par zéro et maximiser la stabilité numérique
// Les traces partielles ti = 1 + 2*(signe selon la diagonale) valent 4*qi²
Quaternion Quaternion::fromRotationMatrix(const Matrix3& R)
{
    float t0 = 1 + R.m[0] + R.m[4] + R.m[8];  // 4a²
    float t1 = 1 + R.m[0] - R.m[4] - R.m[8];  // 4b²
    float t2 = 1 - R.m[0] + R.m[4] - R.m[8];  // 4c²
    float t3 = 1 - R.m[0] - R.m[4] + R.m[8];  // 4d²

    if (t0 >= t1 && t0 >= t2 && t0 >= t3) {
        float a = std::sqrt(t0) / 2;
        return Quaternion(a,
            (R.m[7] - R.m[5]) / (4*a),   // R21 - R12
            (R.m[2] - R.m[6]) / (4*a),   // R02 - R20
            (R.m[3] - R.m[1]) / (4*a));  // R10 - R01
    }
    else if (t1 >= t2 && t1 >= t3) {
        float b = std::sqrt(t1) / 2;
        return Quaternion(
            (R.m[7] - R.m[5]) / (4*b),   // R21 - R12
            b,
            (R.m[3] + R.m[1]) / (4*b),   // R10 + R01
            (R.m[2] + R.m[6]) / (4*b));  // R02 + R20
    }
    else if (t2 >= t3) {
        float c = std::sqrt(t2) / 2;
        return Quaternion(
            (R.m[2] - R.m[6]) / (4*c),   // R02 - R20
            (R.m[3] + R.m[1]) / (4*c),   // R10 + R01
            c,
            (R.m[7] + R.m[5]) / (4*c));  // R21 + R12
    }
    else {
        float d = std::sqrt(t3) / 2;
        return Quaternion(
            (R.m[3] - R.m[1]) / (4*d),   // R10 - R01
            (R.m[2] + R.m[6]) / (4*d),   // R02 + R20
            (R.m[7] + R.m[5]) / (4*d),   // R21 + R12
            d);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Fonctions globales
// ─────────────────────────────────────────────────────────────────────────────

// Compare deux matrices 4x4 composante par composante avec tolérance eps (erreurs flottantes)
bool matricesEgales(const Matrix4& A, const Matrix4& B, float eps)
{
    for (int i = 0; i < 16; ++i)
        if (std::fabs(A.m[i] - B.m[i]) > eps) return false;
    return true;
}

// Construit q = cos(θ/2) + sin(θ/2)*(ax,ay,az) après normalisation de l'axe
// Retourne l'identité si l'axe est nul (norme < 1e-6)
Quaternion makeRotation(float ax, float ay, float az, float angle)
{
    float len = std::sqrt(ax*ax + ay*ay + az*az);
    if (len < 1e-6f) return Quaternion(1, 0, 0, 0);
    ax /= len; ay /= len; az /= len;
    float s = std::sin(angle / 2);
    return Quaternion(std::cos(angle / 2), ax*s, ay*s, az*s);
}

// Rotation via la formule quaternionique : v' = q * p * q*
// p = quaternion pur (0,vx,vy,vz), le résultat est extrait de (r.b, r.c, r.d)
Vector3 rotateByQuaternion(const Vector3& v, const Quaternion& q)
{
    Quaternion p(0, v.x, v.y, v.z);
    Quaternion r = q * p * q.conjugate();
    return Vector3(r.b, r.c, r.d);
}

// Rotation via produit matrice-vecteur : v' = R * v (9 mult + 6 add)
Vector3 rotateByMatrix(const Vector3& v, const Matrix3& R)
{
    return Vector3(
        R.m[0]*v.x + R.m[1]*v.y + R.m[2]*v.z,
        R.m[3]*v.x + R.m[4]*v.y + R.m[5]*v.z,
        R.m[6]*v.x + R.m[7]*v.y + R.m[8]*v.z
    );
}

// Mise à l'échelle non uniforme : étire ou comprime indépendamment sur chaque axe
Vector3 applyScale(const Vector3& v, float sx, float sy, float sz)
{
    return Vector3(v.x * sx, v.y * sy, v.z * sz);
}

// Translation : déplace le point de (tx,ty,tz)
Vector3 applyTranslate(const Vector3& v, float tx, float ty, float tz)
{
    return Vector3(v.x + tx, v.y + ty, v.z + tz);
}

// Cisaillement : déforme l'espace en inclinant chaque axe selon les autres
// hxy = influence de y sur x, hxz = influence de z sur x, etc.
Vector3 applyShear(const Vector3& v, float hxy, float hxz,
                                     float hyx, float hyz,
                                     float hzx, float hzy)
{
    return Vector3(
        v.x + hxy*v.y + hxz*v.z,
        hyx*v.x + v.y + hyz*v.z,
        hzx*v.x + hzy*v.y + v.z
    );
}

// Rotation décentrée autour d'un pivot arbitraire en 3 étapes :
// 1. ramener le pivot à l'origine  2. tourner  3. retransposer
Vector3 rotateAround(const Vector3& v, const Vector3& pivot, const Quaternion& q)
{
    Vector3 local(v.x - pivot.x, v.y - pivot.y, v.z - pivot.z); // ramener à l'origine
    Vector3 rotated = rotateByQuaternion(local, q);               // tourner
    return Vector3(rotated.x + pivot.x, rotated.y + pivot.y, rotated.z + pivot.z); // replacer
}

// Vérifie que la représentation matricielle L(q) est compatible avec les opérations :
// - additive  : L(q1+q2) == L(q1) + L(q2)
// - multiplicative : L(q1*q2) == L(q1) * L(q2)
bool verifierCompatibilite(const Quaternion& q1, const Quaternion& q2)
{
    // M(q1 + q2) == M(q1) + M(q2)
    bool addOk = matricesEgales((q1 + q2).toMatrix(),
                                 q1.toMatrix() + q2.toMatrix());

    // M(q1 * q2) == M(q1) * M(q2)
    bool mulOk = matricesEgales((q1 * q2).toMatrix(),
                                 q1.toMatrix() * q2.toMatrix());

    return addOk && mulOk;
}
