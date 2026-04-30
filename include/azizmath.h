#pragma once

struct Matrix3;
struct Matrix4;

struct Complex
{
    float a;    //réelle
    float b;    //imaginaire

    Complex(float reel = 0.0f, float imag = 0.0f) : a(reel), b(imag) {}

    Complex operator+(const Complex& other) const;
    Complex operator*(const Complex& other) const;
    Complex operator/(const Complex& other) const;
    Complex conjugate() const;
    float module() const;
    float argument() const;
};

struct Vector3
{
    float x, y, z;
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;
};

struct Quaternion
{
    float a, b, c, d;

    Quaternion(float _a = 1, float _b = 0, float _c = 0, float _d = 0) : a(_a), b(_b), c(_c), d(_d) {}

    Quaternion operator+(const Quaternion& other) const;
    Quaternion operator*(const Quaternion& other) const;
    Quaternion conjugate() const;
    float norm() const;
    Quaternion normalize() const;

    // partie b) : quaternion <-> matrice algébrique 4x4
    Matrix4 toMatrix() const;
    static Quaternion fromMatrix(const Matrix4& M);

    // partie c) : quaternion de rotation <-> matrice de rotation 3x3
    Matrix3 toRotationMatrix() const;
    static Quaternion fromRotationMatrix(const Matrix3& R);
};

struct Vector4
{
    float x, y, z, w;
    Vector4(float _x = 0, float _y = 0, float _z = 0, float _w = 0) : x(_x), y(_y), z(_z), w(_w) {}

    float dot(const Vector4& other) const;
};

struct Matrix3 {
    float m[9];

    Matrix3() {
        for (int i = 0; i < 9; ++i) m[i] = 0.0f;
    }

    Matrix3 operator*(const Matrix3& other) const;
    Matrix3 operator+(const Matrix3& other) const;
};

struct Matrix4 {
    float m[16];

    Matrix4() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    }

    Matrix4 operator*(const Matrix4& other) const;
    Matrix4 operator+(const Matrix4& other) const;
};

bool matricesEgales(const Matrix4& A, const Matrix4& B, float eps = 1e-5f);
bool verifierCompatibilite(const Quaternion& q1, const Quaternion& q2);

// partie d) : rotation d'un vecteur 3D
Vector3 rotateByQuaternion(const Vector3& v, const Quaternion& q);
Vector3 rotateByMatrix(const Vector3& v, const Matrix3& R);

// construit le quaternion de rotation depuis un axe (pas forcément unitaire) et un angle
Quaternion makeRotation(float ax, float ay, float az, float angle);

// partie e) : rotation décentrée autour d'un pivot
Vector3 rotateAround(const Vector3& v, const Vector3& pivot, const Quaternion& q);

// partie f) : transformations affines
Vector3 applyScale(const Vector3& v, float sx, float sy, float sz);
Vector3 applyTranslate(const Vector3& v, float tx, float ty, float tz);
// cisaillement : x'=x+hxy*y+hxz*z, y'=hyx*x+y+hyz*z, z'=hzx*x+hzy*y+z
Vector3 applyShear(const Vector3& v, float hxy, float hxz,
                                     float hyx, float hyz,
                                     float hzx, float hzy);
