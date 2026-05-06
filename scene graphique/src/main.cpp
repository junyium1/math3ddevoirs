#define STB_IMAGE_IMPLEMENTATION // active l'implémentation unique de stb_image
#define STBI_NO_SIMD              // désactive les instructions SIMD (compatibilité MSVC)
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "azizmath.h"
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

// Matrice 4x4 colonne-major pour OpenGL (layout natif de glUniformMatrix4fv)
struct Mat4 { float m[16] = {}; };

// Construit la matrice vue (caméra orbitale) : base orthonormée (right, up, -forward) + translation
static Mat4 lookAt(Vector3 eye, Vector3 center) {
    Vector3 f(center.x - eye.x, center.y - eye.y, center.z - eye.z);
    float fl = sqrtf(f.dot(f)); f = Vector3(f.x / fl, f.y / fl, f.z / fl);
    Vector3 r = f.cross(Vector3(0, 1, 0));
    float rl = sqrtf(r.dot(r)); r = Vector3(r.x / rl, r.y / rl, r.z / rl);
    Vector3 u = r.cross(f);
    Mat4 m;
    m.m[0] = r.x; m.m[4] = r.y; m.m[8] = r.z; m.m[12] = -(r.dot(eye));
    m.m[1] = u.x; m.m[5] = u.y; m.m[9] = u.z; m.m[13] = -(u.dot(eye));
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z; m.m[14] = (f.dot(eye));
    m.m[15] = 1.f;
    return m;
}

// Matrice de projection perspective : transforme le frustum en cube NDC [-1,1]
// fov en radians, near/far = plans de clipping
static Mat4 perspective(float fov, float aspect, float near, float far) {
    float f = 1.f / tanf(fov * 0.5f);
    Mat4 r;
    r.m[0] = f / aspect; r.m[5] = f;
    r.m[10] = (far + near) / (near - far); r.m[11] = -1.f;
    r.m[14] = (2.f * far * near) / (near - far);
    return r;
}

// Construit la matrice modèle TRS depuis une matrice de rotation 3x3, une translation et un scale uniforme
static Mat4 makeModel(const Matrix3& R, float tx, float ty, float tz, float s = 1.f) {
    Mat4 m;
    m.m[0] = R.m[0] * s; m.m[4] = R.m[1] * s; m.m[8] = R.m[2] * s; m.m[12] = tx;
    m.m[1] = R.m[3] * s; m.m[5] = R.m[4] * s; m.m[9] = R.m[5] * s; m.m[13] = ty;
    m.m[2] = R.m[6] * s; m.m[6] = R.m[7] * s; m.m[10] = R.m[8] * s; m.m[14] = tz;
    m.m[3] = 0;        m.m[7] = 0;         m.m[11] = 0;         m.m[15] = 1;
    return m;
}

// Lit un fichier texte entier et retourne son contenu (utilisé pour charger les shaders GLSL)
static std::string readFile(const char* path) {
    std::ifstream f(path);
    if (!f) { fprintf(stderr, "Err %s\n", path); return ""; }
    std::ostringstream ss; ss << f.rdbuf(); return ss.str();
}

// Compile un shader GLSL (vertex ou fragment), affiche les erreurs sur stderr si échec
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr); glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[512]; glGetShaderInfoLog(s, 512, nullptr, log); fprintf(stderr, "S: %s\n", log); }
    return s;
}

// Lie vertex + fragment shader en un programme OpenGL, supprime les shaders intermédiaires
static GLuint loadProgram(const char* vp, const char* fp) {
    std::string vs = readFile(vp), fs = readFile(fp);
    GLuint v = compileShader(GL_VERTEX_SHADER, vs.c_str()), f = compileShader(GL_FRAGMENT_SHADER, fs.c_str());
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f); glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// Génère un UV-sphere (stacks x slices) : positions, normales, coordonnées UV
// Format entrelacé : [px,py,pz, nx,ny,nz, u,v] par vertex
static void generateSphere(int stacks, int slices, std::vector<float>& v, std::vector<unsigned>& idx) {
    for (int i = 0; i <= stacks; ++i) {
        float phi = (float)M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.f * (float)M_PI * j / slices;
            float x = sinf(phi) * cosf(theta), y = cosf(phi), z = sinf(phi) * sinf(theta);
            v.insert(v.end(), { x,y,z, x,y,z, (float)j / slices,(float)i / stacks });
        }
    }
    for (int i = 0; i < stacks; ++i)
        for (int j = 0; j < slices; ++j) {
            int a = i * (slices + 1) + j, b = a + slices + 1;
            idx.insert(idx.end(), { (unsigned)a,(unsigned)b,(unsigned)(a + 1),(unsigned)b,(unsigned)(b + 1),(unsigned)(a + 1) });
        }
}

// Génère un anneau plat (disque creux) dans le plan XZ entre rayon ri (intérieur) et ro (extérieur)
// Utilisé pour les anneaux de Saturne et Uranus
static void generateRing(float ri, float ro, int slices, std::vector<float>& v, std::vector<unsigned>& idx) {
    for (int j = 0; j <= slices; ++j) {
        float theta = 2.f * (float)M_PI * j / slices, ct = cosf(theta), st = sinf(theta), u = (float)j / slices;
        v.insert(v.end(), { ri * ct,0,ri * st, 0,1,0, u,0 });
        v.insert(v.end(), { ro * ct,0,ro * st, 0,1,0, u,1 });
    }
    for (int j = 0; j < slices; ++j) {
        unsigned a = j * 2, b = j * 2 + 1, c = j * 2 + 2, d = j * 2 + 3;
        idx.insert(idx.end(), { a,b,c,b,d,c });
    }
}

// Upload les données vertex+indices sur le GPU et configure les vertex attribs (pos/normal/uv)
// Retourne le VAO, écrit vbo et ebo passés par référence
static GLuint uploadMesh(const std::vector<float>& v, const std::vector<unsigned>& idx, GLuint& vbo, GLuint& ebo) {
    GLuint vao; int stride = 8 * sizeof(float);
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                   glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    return vao;
}

// VAO/VBO/EBO + nombre d'indices pour un tracé d'orbite en pointillés
struct OrbitMesh { GLuint vao, vbo, ebo; int count; };

// Construit l'orbite en pointillés d'une planète : N points sur un cercle incliné de 'incl' radians
// Le tracé est généré avec makeRotation (quaternion), conformément au cours
static OrbitMesh buildOrbit(float radius, float incl, int N) {
    std::vector<float> verts; std::vector<unsigned> idx;
    float si = sinf(incl), ci = cosf(incl);
    for (int j = 0; j < N; j++) {
        Quaternion q = makeRotation(si, ci, 0, 2.f * (float)M_PI * j / N);
        Vector3 pt = rotateByQuaternion(Vector3(radius, 0, 0), q);
        verts.insert(verts.end(), { pt.x,pt.y,pt.z, 0,1,0, 0,0 });
    }
    const int dash = 1, gap = 6, step = dash + gap;
    for (int j = 0; j < N; j += step)
        for (int k = 0; k < dash; k++) { idx.push_back((j + k) % N); idx.push_back((j + k + 1) % N); }
    OrbitMesh om; int stride = 8 * sizeof(float);
    glGenVertexArrays(1, &om.vao); glGenBuffers(1, &om.vbo); glGenBuffers(1, &om.ebo);
    glBindVertexArray(om.vao);
    glBindBuffer(GL_ARRAY_BUFFER, om.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, om.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                   glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);
    om.count = (int)idx.size(); return om;
}

// Charge une image disque (JPG/PNG) via stb_image, la téléverse sur le GPU avec mipmaps
// Retourne 0 et affiche une erreur si le fichier est introuvable
static GLuint loadTexture(const char* path) {
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    if (!data) { fprintf(stderr, "Err: %s\n", path); return 0; }
    GLuint tex; glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data); return tex;
}

// Variables globales de la caméra orbitale (partagées par les callbacks GLFW)
static float camYaw = 0.4f, camPitch = 0.5f, camDist = 35.f;
static double lastX = 0, lastY = 0;
static bool dragging = false;
// Callback scroll : zoom en modifiant la distance orbitale (facteur exponentiel)
static void cbScroll(GLFWwindow*, double, double dy) { camDist = fmaxf(0.1f, camDist * powf(0.88f, (float)dy)); }
// Callback clic : active/désactive le drag de caméra sur le bouton gauche
static void cbMouse(GLFWwindow*, int btn, int action, int) { dragging = (btn == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS); }
// Callback mouvement : met à jour yaw et pitch quand le bouton gauche est enfoncé
static void cbCursor(GLFWwindow*, double x, double y) {
    if (dragging) { camYaw -= (float)(x - lastX) * 0.005f; camPitch += fmaxf(-1.4f, fminf(1.4f, (float)(y - lastY) * 0.005f + camPitch)) - camPitch; }
    lastX = x; lastY = y;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* win = glfwCreateWindow(1400, 800, "Solar", nullptr, nullptr);
    glfwMakeContextCurrent(win); glfwSwapInterval(1);
    glfwSetScrollCallback(win, cbScroll); glfwSetMouseButtonCallback(win, cbMouse); glfwSetCursorPosCallback(win, cbCursor);

    glewExperimental = GL_TRUE; glewInit();
    glEnable(GL_DEPTH_TEST); glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true); ImGui_ImplOpenGL3_Init("#version 330");

    GLuint prog = loadProgram("shaders/shader.vert", "shaders/shader.frag");

    GLuint texSun = loadTexture("textures/2k_sun.jpg");
    GLuint texPlanets[8] = {
        loadTexture("textures/2k_mercury.jpg"), loadTexture("textures/2k_venus.jpg"),
        loadTexture("textures/2k_earth.jpg"),   loadTexture("textures/2k_mars.jpg"),
        loadTexture("textures/2k_jupiter.jpg"), loadTexture("textures/2k_saturn.jpg"),
        loadTexture("textures/2k_uranus.jpg"),  loadTexture("textures/2k_neptune.jpg"),
    };
    GLuint texMoon = loadTexture("textures/2k_moon.jpg");
    GLuint texSaturnRing = loadTexture("textures/saturnrings.jpg");
    GLuint texUranusRing = loadTexture("textures/Uranus_rings.png");
    GLint locTex = glGetUniformLocation(prog, "uTexture");

    std::vector<float> sv, rv; std::vector<unsigned> si, ri;
    generateSphere(32, 32, sv, si); generateRing(1.1f, 1.9f, 64, rv, ri);
    GLuint svbo, sebo, rvbo, rebo;
    GLuint sVao = uploadMesh(sv, si, svbo, sebo), rVao = uploadMesh(rv, ri, rvbo, rebo);

    GLint locModel = glGetUniformLocation(prog, "uModel"), locView = glGetUniformLocation(prog, "uView");
    GLint locProj = glGetUniformLocation(prog, "uProj"), locColor = glGetUniformLocation(prog, "uColor");
    GLint locUseTex = glGetUniformLocation(prog, "uUseTexture"), locLight = glGetUniformLocation(prog, "uLightPos");
    GLint locEye = glGetUniformLocation(prog, "uViewPos"), locEmit = glGetUniformLocation(prog, "uEmissive");
    GLint locLightInt = glGetUniformLocation(prog, "uLightIntensity"), locAmbInt = glGetUniformLocation(prog, "uAmbientIntensity");

    // Paramètres orbitaux et physiques de chaque planète
    struct Planet {
        float orbitRadius;    // distance au Soleil (unités de scène)
        float orbitSpeed;     // vitesse angulaire orbitale (rad/s en temps de scène)
        float orbInclination; // inclinaison du plan orbital (rad), axe makeRotation(sin,cos,0,angle)
        float spinSpeed;      // vitesse de rotation propre (rad/s)
        float axialTilt;      // inclinaison axiale (rad), axe de rotation propre
        float size;           // rayon visuel de la planète
        float r, g, b;        // couleur de fallback si texture absente
    };
    // spinSpeed = orbitSpeed * (periode_orbitale / periode_rotation) * facteur_visuel
    // Facteur visuel : Terre = 2.0 rad/s de spin visible (365 rotations/orbite en realite)
    // orbInclination et axialTilt passés à makeRotation() d'azizmath
    Planet planets[] = {
        //  radius  ospd    oinc     spin     atilt    size   r      g      b
        {  3.5f, 4.15f, 0.122f,  0.034f,  0.01f,   0.18f, 0.76f,0.65f,0.58f }, // Mercure  (1.5 rot/orbite)
        {  5.5f, 1.62f, 0.059f,  0.008f,  3.096f,  0.30f, 0.95f,0.78f,0.42f }, // Vénus    (rétrograde)
        {  7.5f, 1.00f, 0.000f,  2.000f,  0.409f,  0.32f, 0.22f,0.48f,0.90f }, // Terre    (référence)
        {  9.8f, 0.53f, 0.032f,  1.940f,  0.440f,  0.22f, 0.78f,0.35f,0.18f }, // Mars
        { 13.5f, 0.08f, 0.023f,  4.580f,  0.054f,  0.90f, 0.82f,0.72f,0.56f }, // Jupiter  (géante gazeuse)
        { 17.5f, 0.03f, 0.044f,  3.980f,  0.467f,  0.75f, 0.90f,0.80f,0.58f }, // Saturne  (anneaux)
        { 21.0f, 0.01f, 0.014f,  2.340f,  1.706f,  0.50f, 0.52f,0.80f,0.95f }, // Uranus   (tilt ~98°)
        { 24.0f, 0.006f,0.031f,  2.940f,  0.494f,  0.48f, 0.20f,0.35f,0.85f }, // Neptune
    };

    OrbitMesh orbits[8];
    for (int i = 0; i < 8; i++) orbits[i] = buildOrbit(planets[i].orbitRadius, planets[i].orbInclination, 400);

    float speedMult = 1.0f;
    int selectedPlanet = -1;
    const char* planetNames[] = { "Mercure","Venus","Terre","Mars","Jupiter","Saturne","Uranus","Neptune" };
    const float closeupDist[] = { 5.0f,0.5f,0.8f,0.9f,0.6f,2.5f,2.2f,1.5f,1.5f,0.3f };

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        int w, h; glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.01f, 0.01f, 0.04f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float t = (float)glfwGetTime();

        // Calcul des positions orbitales via quaternion de rotation incliné
        // q = makeRotation(sin(incl), cos(incl), 0, t*speed) → rotate le vecteur (radius,0,0)
        Vector3 planetPos[8]; Quaternion orbitQ[8];
        for (int i = 0; i < 8; i++) {
            auto& p = planets[i];
            Quaternion q = makeRotation(sinf(p.orbInclination), cosf(p.orbInclination), 0, t * p.orbitSpeed * speedMult);
            orbitQ[i] = q; planetPos[i] = rotateByQuaternion(Vector3(p.orbitRadius, 0, 0), q);
        }
        // Lune : orbite autour de la Terre avec une légère inclinaison
        Quaternion qMoonOrbit = makeRotation(0, 1, 0.05f, t * 0.073f * speedMult);
        Vector3 moonPos = applyTranslate(rotateByQuaternion(Vector3(0.7f, 0, 0), qMoonOrbit),
            planetPos[2].x, planetPos[2].y, planetPos[2].z);

        // Cible de la caméra : centre du système ou planète sélectionnée
        Vector3 camTarget(0, 0, 0);
        if (selectedPlanet >= 0 && selectedPlanet < 8) camTarget = planetPos[selectedPlanet];
        else if (selectedPlanet == 9)              camTarget = moonPos;

        // Position de l'oeil : quaternion caméra (yaw * pitch) appliqué à (0,0,dist) + target
        Quaternion qCam = makeRotation(0, 1, 0, camYaw) * makeRotation(1, 0, 0, -camPitch);
        Vector3 eye = applyTranslate(rotateByQuaternion(Vector3(0, 0, camDist), qCam),
            camTarget.x, camTarget.y, camTarget.z);

        Mat4 view = lookAt(eye, camTarget);
        Mat4 proj = perspective(60.f * (float)M_PI / 180.f, (float)w / h, 0.1f, 1000.f);

        glUseProgram(prog);
        glUniformMatrix4fv(locView, 1, GL_FALSE, view.m); glUniformMatrix4fv(locProj, 1, GL_FALSE, proj.m);
        glUniform3f(locLight, 0, 0, 0); glUniform3f(locEye, eye.x, eye.y, eye.z);
        glUniform1f(locLightInt, 5.5f); glUniform1f(locAmbInt, 0.15f);
        glActiveTexture(GL_TEXTURE0); glUniform1i(locTex, 0); glUniform1i(locUseTex, 1);

        // Soleil : émissif (pas d'ombrage), tourne sur lui-même à 0.3 rad/s
        glBindVertexArray(sVao);
        {
            Mat4 model = makeModel(makeRotation(0, 1, 0, t * 0.3f).toRotationMatrix(), 0, 0, 0, 1.5f);
            glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m);
            glUniform1i(locEmit, 1); glBindTexture(GL_TEXTURE_2D, texSun);
            glDrawElements(GL_TRIANGLES, (GLsizei)si.size(), GL_UNSIGNED_INT, 0);
        }
        glUniform1i(locEmit, 0);

        // Orbites en pointillés (mode GL_LINES) avec matrice identité (référentiel monde)
        {
            Mat4 id; id.m[0] = id.m[5] = id.m[10] = id.m[15] = 1.f;
            glUniformMatrix4fv(locModel, 1, GL_FALSE, id.m);
            glUniform1i(locEmit, 1); glUniform1i(locUseTex, 0); glUniform3f(locColor, 0.35f, 0.38f, 0.45f);
            for (int i = 0; i < 8; i++) { glBindVertexArray(orbits[i].vao); glDrawElements(GL_LINES, orbits[i].count, GL_UNSIGNED_INT, 0); }
            glUniform1i(locEmit, 0); glUniform1i(locUseTex, 1); glBindVertexArray(sVao);
        }

        // Planètes : spinQ = quaternion de rotation propre (axial tilt + spin temp)
        Quaternion spinQ[8];
        for (int i = 0; i < 8; i++) {
            auto& p = planets[i];
            spinQ[i] = makeRotation(sinf(p.axialTilt), cosf(p.axialTilt), 0, t * p.spinSpeed * speedMult);
            Mat4 model = makeModel(spinQ[i].toRotationMatrix(), planetPos[i].x, planetPos[i].y, planetPos[i].z, p.size);
            glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m);
            glBindTexture(GL_TEXTURE_2D, texPlanets[i]);
            glDrawElements(GL_TRIANGLES, (GLsizei)si.size(), GL_UNSIGNED_INT, 0);
        }

        // Lune : petite sphère qui orbite autour de la Terre
        {
            Mat4 model = makeModel(makeRotation(0, 1, 0, t * 0.073f * speedMult).toRotationMatrix(), moonPos.x, moonPos.y, moonPos.z, 0.09f);
            glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m);
            glBindTexture(GL_TEXTURE_2D, texMoon);
            glDrawElements(GL_TRIANGLES, (GLsizei)si.size(), GL_UNSIGNED_INT, 0);
        }

        // Anneaux : Saturne (i=5) et Uranus (i=6) — plan incliné selon l'axial tilt via makeRotation
        for (int i : {6, 5}) {
            float tilt = planets[i].axialTilt;
            Mat4 model = makeModel(makeRotation(0, 0, 1, tilt).toRotationMatrix(), planetPos[i].x, planetPos[i].y, planetPos[i].z, planets[i].size);
            glUniformMatrix4fv(locModel, 1, GL_FALSE, model.m); glUniform1i(locUseTex, 1);
            glBindTexture(GL_TEXTURE_2D, i == 6 ? texUranusRing : texSaturnRing);
            glBindVertexArray(rVao); glDrawElements(GL_TRIANGLES, (GLsizei)ri.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(sVao);
        }

        glUniform1i(locUseTex, 1);

        // UI
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();
        ImGui::SetNextWindowPos({ 10,10 }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ 340,0 }, ImGuiCond_Always);
        ImGui::Begin("DEBUG", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("FPS  %.1f", ImGui::GetIO().Framerate);
        ImGui::Separator();
        ImGui::SliderFloat("Speed", &speedMult, 0.01f, 100.0f);
        if (ImGui::Button("Reset")) speedMult = 1.f;
        ImGui::Separator();

        if (ImGui::CollapsingHeader("CAM", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Sys")) { selectedPlanet = -1; camDist = 35.f; } ImGui::SameLine();
            if (ImGui::Button("Sun")) { selectedPlanet = -2; camDist = 5.f; }
            for (int i = 0; i < 8; i++) {
                if (i % 4 == 0) ImGui::NewLine();
                if (ImGui::Button(planetNames[i])) { selectedPlanet = i; camDist = closeupDist[i + 1]; } ImGui::SameLine();
            }
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("QUATERNIONS")) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.85f, 0.3f, 1.f));
            ImGui::TextUnformatted("         a(w)       b(x)       c(y)       d(z)");
            ImGui::PopStyleColor();
            for (int i = 0; i < 8; i++) {
                char label[64]; snprintf(label, sizeof(label), "%s##quat", planetNames[i]);
                if (ImGui::TreeNode(label)) {
                    ImGui::TextDisabled("Orbite  ");
                    ImGui::SameLine();
                    ImGui::Text("% .4f  % .4f  % .4f  % .4f",
                        orbitQ[i].a, orbitQ[i].b, orbitQ[i].c, orbitQ[i].d);
                    ImGui::TextDisabled("Spin    ");
                    ImGui::SameLine();
                    ImGui::Text("% .4f  % .4f  % .4f  % .4f",
                        spinQ[i].a, spinQ[i].b, spinQ[i].c, spinQ[i].d);
                    ImGui::TreePop();
                }
            }
            if (ImGui::TreeNode("Lune##quat")) {
                ImGui::TextDisabled("Orbite  ");
                ImGui::SameLine();
                ImGui::Text("% .4f  % .4f  % .4f  % .4f",
                    qMoonOrbit.a, qMoonOrbit.b, qMoonOrbit.c, qMoonOrbit.d);
                ImGui::TreePop();
            }
        }
        ImGui::End(); ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
    glfwTerminate(); return 0;
}