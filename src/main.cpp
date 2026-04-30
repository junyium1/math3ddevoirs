#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cstdio>
#include <cmath>
#include "azizmath.h"

static const Vector3 CUBE[8] = {
    {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
    {-1,-1, 1},{1,-1, 1},{1,1, 1},{-1,1, 1}
};
static const int EDGES[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

// ── Caméra orbite ─────────────────────────────────────────────────────────────
struct Camera {
    float yaw   =  0.6f;
    float pitch =  0.4f;
    float dist  = 12.0f;
    float panX  =  0.0f;
    float panY  =  0.0f;
    float panZ  =  0.0f;
};

// Projette un point 3D en coordonnées écran sur le canvas.
// Retourne false si le point est derrière la caméra.
static bool projectCam(const Vector3& p, const Camera& cam,
                       ImVec2 cPos, ImVec2 cSize, ImVec2& out)
{
    float cp = cosf(cam.pitch), sp = sinf(cam.pitch);
    float cy = cosf(cam.yaw),   sy = sinf(cam.yaw);

    // Position de l'oeil
    float ex = cam.panX + cam.dist * cp * sy;
    float ey = cam.panY + cam.dist * sp;
    float ez = cam.panZ + cam.dist * cp * cy;

    // Vecteur d -> point vu depuis l'oeil
    float dx = p.x - ex, dy = p.y - ey, dz = p.z - ez;

    // Passage en espace vue (base : right, up, forward)
    // right   = (cy,       0,       -sy)
    // up      = (-sy*sp,   cp,      -cy*sp)
    // forward = (-cp*sy,  -sp,      -cp*cy)
    float vx =  cy*dx                      - sy*dz;
    float vy = -sy*sp*dx  + cp*dy  - cy*sp*dz;
    float vz = -cp*sy*dx  - sp*dy  - cp*cy*dz; // profondeur positive = devant

    if (vz <= 0.01f) return false;

    float fov    = 60.0f * 3.14159265f / 180.0f;
    float f      = 1.0f / tanf(fov * 0.5f);
    float aspect = cSize.x / cSize.y;

    float nx = (vx / vz) * f / aspect;
    float ny = (vy / vz) * f;

    out = ImVec2(cPos.x + cSize.x * (0.5f + nx * 0.5f),
                 cPos.y + cSize.y * (0.5f - ny * 0.5f));
    return true;
}

static void drawLine(ImDrawList* dl, const Vector3& a, const Vector3& b,
                     const Camera& cam, ImVec2 cPos, ImVec2 cSize,
                     ImU32 col, float thickness = 1.5f)
{
    ImVec2 pa, pb;
    if (projectCam(a, cam, cPos, cSize, pa) &&
        projectCam(b, cam, cPos, cSize, pb))
        dl->AddLine(pa, pb, col, thickness);
}

static void drawPoint(ImDrawList* dl, const Vector3& p, const Camera& cam,
                      ImVec2 cPos, ImVec2 cSize,
                      ImU32 col, float r, const char* label = nullptr)
{
    ImVec2 sp;
    if (!projectCam(p, cam, cPos, cSize, sp)) return;
    dl->AddCircleFilled(sp, r, col);
    if (label)
        dl->AddText(ImVec2(sp.x + r + 4, sp.y - 8), col, label);
}

static void glfw_error(int e, const char* d) { fprintf(stderr, "GLFW %d: %s\n", e, d); }

int main()
{
    glfwSetErrorCallback(glfw_error);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1400, 800, "Rotations 3D", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 32.0f);
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float  axis[3]   = {0, 1, 0};
    float  angle     = 0.0f;
    float  axis2[3]  = {1, 0, 0};
    float  angle2    = 0.0f;
    float  pt[3]     = {1, 0, 0};
    float  pivot[3]  = {2, 0, 0};
    float  sc[3]     = {1, 1, 1};       // scale
    float  tr[3]     = {0, 0, 0};       // translation
    float  shear[6]  = {0,0,0,0,0,0};  // hxy,hxz,hyx,hyz,hzx,hzy
    bool   animate   = false;
    bool   useCompose  = true;
    bool   useScale    = false;
    bool   useTranslate= false;
    bool   useShear    = false;
    Camera cam;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (animate)
            angle = fmodf((float)glfwGetTime(), 2.0f * 3.14159265f);

        Quaternion q1  = makeRotation(axis[0],  axis[1],  axis[2],  angle);
        Quaternion q2  = makeRotation(axis2[0], axis2[1], axis2[2], angle2);
        Quaternion qC  = q1 * q2;   // composée : q2 d'abord, puis q1
        Matrix3    R   = q1.toRotationMatrix();

        Vector3    v(pt[0], pt[1], pt[2]);
        Vector3    piv(pivot[0], pivot[1], pivot[2]);
        Vector3    rQ   = rotateByQuaternion(v, q1);
        Vector3    rM   = rotateByMatrix(v, R);
        Vector3    rDec = rotateAround(v, piv, q1);
        float dx = rQ.x-rM.x, dy = rQ.y-rM.y, dz = rQ.z-rM.z;
        float err = sqrtf(dx*dx + dy*dy + dz*dz);

        // Transformée complète : scale → rotation composée → shear → translate
        auto applyAll = [&](Vector3 p) {
            if (useScale)     p = applyScale(p, sc[0], sc[1], sc[2]);
            if (useCompose)   p = rotateByQuaternion(p, qC);
            if (useShear)     p = applyShear(p, shear[0],shear[1],shear[2],shear[3],shear[4],shear[5]);
            if (useTranslate) p = applyTranslate(p, tr[0], tr[1], tr[2]);
            return p;
        };

        ImGuiIO& io = ImGui::GetIO();
        float W = io.DisplaySize.x, H = io.DisplaySize.y;
        float leftW = W * 0.38f;

        constexpr ImGuiWindowFlags PINNED =
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

        // ── Panneau gauche ────────────────────────────────────────────────
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({leftW, H});
        ImGui::Begin("Controles", nullptr, PINNED);

        ImGui::SeparatorText("Axe de rotation");
        ImGui::SliderFloat("x##ax", &axis[0], -1.0f, 1.0f);
        ImGui::SliderFloat("y##ax", &axis[1], -1.0f, 1.0f);
        ImGui::SliderFloat("z##ax", &axis[2], -1.0f, 1.0f);

        ImGui::SeparatorText("Angle");
        ImGui::SliderFloat("angle (rad)", &angle, 0.0f, 6.2832f);
        ImGui::Checkbox("Animer", &animate);

        ImGui::SeparatorText("Point v a tourner");
        ImGui::InputFloat("vx", &pt[0], 0.1f);
        ImGui::InputFloat("vy", &pt[1], 0.1f);
        ImGui::InputFloat("vz", &pt[2], 0.1f);

        ImGui::SeparatorText("Pivot (rotation decentree)");
        ImGui::InputFloat("px", &pivot[0], 0.1f);
        ImGui::InputFloat("py", &pivot[1], 0.1f);
        ImGui::InputFloat("pz", &pivot[2], 0.1f);

        ImGui::SeparatorText("Quaternion");
        ImGui::Text("q = (%.3f, %.3fi, %.3fj, %.3fk)", q1.a, q1.b, q1.c, q1.d);
        ImGui::Text("||q|| = %.6f", q1.norm());

        ImGui::SeparatorText("Resultats");
        ImGui::Text("Via quaternion  : (%.4f, %.4f, %.4f)", rQ.x, rQ.y, rQ.z);
        ImGui::Text("Via matrice     : (%.4f, %.4f, %.4f)", rM.x, rM.y, rM.z);
        if (err < 1e-4f)
            ImGui::TextColored({0,1,0,1}, "Erreur : %.2e  OK", err);
        else
            ImGui::TextColored({1,0,0,1}, "Erreur : %.2e", err);
        ImGui::Text("Decentree pivot : (%.4f, %.4f, %.4f)", rDec.x, rDec.y, rDec.z);

        ImGui::SeparatorText("Exemple cours (p.6 ex.2)");
        {
            float inv3 = 1.0f / sqrtf(3.0f);
            Quaternion qEx = makeRotation(inv3, inv3, inv3, 2.0f * 3.14159265f / 3.0f);
            ImGui::Text("q = (%.4f, %.4f, %.4f, %.4f)", qEx.a, qEx.b, qEx.c, qEx.d);
            static float ep[3] = {1, 2, 3};
            ImGui::InputFloat("a##ex", &ep[0], 0.1f);
            ImGui::InputFloat("b##ex", &ep[1], 0.1f);
            ImGui::InputFloat("c##ex", &ep[2], 0.1f);
            ImGui::TextDisabled("Attendu (c,a,b) = (%.2f, %.2f, %.2f)", ep[2], ep[0], ep[1]);
            Vector3 exR = rotateByQuaternion({ep[0],ep[1],ep[2]}, qEx);
            ImGui::Text("Resultat : (%.4f, %.4f, %.4f)", exR.x, exR.y, exR.z);
        }

        ImGui::SeparatorText("f) Composee de rotations");
        ImGui::TextDisabled("q_total = q1 * q2  (q2 d'abord, puis q1)");
        ImGui::Checkbox("Activer composition##c", &useCompose);
        ImGui::SliderFloat("x2", &axis2[0], -1.0f, 1.0f);
        ImGui::SliderFloat("y2", &axis2[1], -1.0f, 1.0f);
        ImGui::SliderFloat("z2", &axis2[2], -1.0f, 1.0f);
        ImGui::SliderFloat("angle2 (rad)", &angle2, 0.0f, 6.2832f);
        ImGui::Text("qC = (%.3f, %.3fi, %.3fj, %.3fk)", qC.a, qC.b, qC.c, qC.d);

        ImGui::SeparatorText("Scaling");
        ImGui::Checkbox("Activer##s", &useScale);
        ImGui::SliderFloat("sx", &sc[0], 0.1f, 3.0f);
        ImGui::SliderFloat("sy", &sc[1], 0.1f, 3.0f);
        ImGui::SliderFloat("sz", &sc[2], 0.1f, 3.0f);

        ImGui::SeparatorText("Translation");
        ImGui::Checkbox("Activer##t", &useTranslate);
        ImGui::InputFloat("tx", &tr[0], 0.1f);
        ImGui::InputFloat("ty", &tr[1], 0.1f);
        ImGui::InputFloat("tz", &tr[2], 0.1f);

        ImGui::SeparatorText("Cisaillement");
        ImGui::Checkbox("Activer##sh", &useShear);
        ImGui::SliderFloat("hxy", &shear[0], -2.0f, 2.0f);
        ImGui::SliderFloat("hxz", &shear[1], -2.0f, 2.0f);
        ImGui::SliderFloat("hyz", &shear[3], -2.0f, 2.0f);

        ImGui::SeparatorText("Operations");
        ImGui::TextDisabled("Matrice R*v       :  9 mult +  6 add");
        ImGui::TextDisabled("Quaternion q*p*q* : 28 mult + 21 add");
        ImGui::TextDisabled("Composer rotations: 16 mult (quat) vs 27 (mat)");

        ImGui::End();

        // ── Panneau droit : viewport 3D interactif ────────────────────────
        ImGui::SetNextWindowPos({leftW, 0});
        ImGui::SetNextWindowSize({W - leftW, H});
        ImGui::Begin("Viewport 3D", nullptr, PINNED | ImGuiWindowFlags_NoScrollbar);

        ImVec2 cPos  = ImGui::GetCursorScreenPos();
        ImVec2 cSize = ImGui::GetContentRegionAvail();

        // Bouton invisible qui capture la souris
        ImGui::InvisibleButton("vp", cSize,
            ImGuiButtonFlags_MouseButtonLeft  |
            ImGuiButtonFlags_MouseButtonRight |
            ImGuiButtonFlags_MouseButtonMiddle);

        bool hovered = ImGui::IsItemHovered();
        bool active  = ImGui::IsItemActive();

        // Zoom (scroll)
        if (hovered && io.MouseWheel != 0.0f)
            cam.dist = fmaxf(1.0f, cam.dist * powf(0.88f, io.MouseWheel));

        if (active) {
            float cp = cosf(cam.pitch), sp = sinf(cam.pitch);
            float cy = cosf(cam.yaw),   sy = sinf(cam.yaw);

            // Clic gauche → orbite
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                cam.yaw   -= io.MouseDelta.x * 0.006f;
                cam.pitch += io.MouseDelta.y * 0.006f;
                cam.pitch  = fmaxf(-1.4f, fminf(1.4f, cam.pitch));
            }
            // Clic droit → pan (dans le plan right/up de la caméra)
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                float s = cam.dist * 0.0015f;
                cam.panX -= ( cy * io.MouseDelta.x - (-sy*sp) * io.MouseDelta.y) * s;
                cam.panY -= (                          cp      * io.MouseDelta.y) * s;
                cam.panZ -= ((-sy)* io.MouseDelta.x - (-cy*sp)* io.MouseDelta.y) * s;
            }
        }

        // ── Dessin ───────────────────────────────────────────────────────
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(cPos, {cPos.x+cSize.x, cPos.y+cSize.y}, IM_COL32(18,18,28,255));

        // Axes du monde
        drawLine(dl, {0,0,0}, {3,0,0}, cam, cPos, cSize, IM_COL32(255, 60, 60, 200), 2.0f);
        drawLine(dl, {0,0,0}, {0,3,0}, cam, cPos, cSize, IM_COL32( 60,255, 60, 200), 2.0f);
        drawLine(dl, {0,0,0}, {0,0,3}, cam, cPos, cSize, IM_COL32( 60,120,255, 200), 2.0f);
        drawPoint(dl, {3,0,0}, cam, cPos, cSize, IM_COL32(255, 60, 60,255), 4, "X");
        drawPoint(dl, {0,3,0}, cam, cPos, cSize, IM_COL32( 60,255, 60,255), 4, "Y");
        drawPoint(dl, {0,0,3}, cam, cPos, cSize, IM_COL32( 60,120,255,255), 4, "Z");

        // Cube initial (gris fantome)
        for (auto& e : EDGES)
            drawLine(dl, CUBE[e[0]], CUBE[e[1]], cam, cPos, cSize, IM_COL32(70,70,90,160), 1.0f);

        // Cube rotaté centré (bleu)
        for (auto& e : EDGES) {
            drawLine(dl,
                rotateByMatrix(CUBE[e[0]], R),
                rotateByMatrix(CUBE[e[1]], R),
                cam, cPos, cSize, IM_COL32(80,160,255,220), 1.8f);
        }

        // Cube rotaté décentré (orange)
        for (auto& e : EDGES) {
            drawLine(dl,
                rotateAround(CUBE[e[0]], piv, q1),
                rotateAround(CUBE[e[1]], piv, q1),
                cam, cPos, cSize, IM_COL32(255,140,30,200), 1.5f);
        }

        // Cube avec toutes les transformations f) (vert)
        for (auto& e : EDGES) {
            drawLine(dl,
                applyAll(CUBE[e[0]]),
                applyAll(CUBE[e[1]]),
                cam, cPos, cSize, IM_COL32(80,255,120,230), 2.0f);
        }

        // Pivot (croix jaune)
        {
            ImVec2 pp;
            if (projectCam(piv, cam, cPos, cSize, pp)) {
                dl->AddLine({pp.x-10,pp.y}, {pp.x+10,pp.y}, IM_COL32(255,220,0,255), 2.5f);
                dl->AddLine({pp.x,pp.y-10}, {pp.x,pp.y+10}, IM_COL32(255,220,0,255), 2.5f);
                dl->AddText({pp.x+12,pp.y-10}, IM_COL32(255,220,0,255), "pivot");
            }
        }

        // Point v original (vert)
        drawPoint(dl, v,    cam, cPos, cSize, IM_COL32( 80,255, 80,255), 7, "v");
        // v' rotation centrée (bleu clair)
        drawPoint(dl, rM,   cam, cPos, cSize, IM_COL32(120,200,255,255), 7, "v' (centree)");
        // v' rotation décentrée (orange)
        drawPoint(dl, rDec, cam, cPos, cSize, IM_COL32(255,160, 40,255), 7, "v' (decentree)");

        // Legende
        float lx = cPos.x + 12, ly = cPos.y + 12;
        auto legend = [&](ImU32 col, const char* text) {
            dl->AddRectFilled({lx,ly+2},{lx+18,ly+14}, col);
            dl->AddText({lx+24,ly}, IM_COL32(220,220,220,255), text);
            ly += 22;
        };
        legend(IM_COL32(70,70,90,200),   "position initiale");
        legend(IM_COL32(80,160,255,220), "rotation q1 centree");
        legend(IM_COL32(255,140,30,200), "rotation q1 decentree pivot");
        legend(IM_COL32(80,255,120,230), "f) toutes transformations (vert)");

        // Hint
        dl->AddText({cPos.x+12, cPos.y+cSize.y-22},
            IM_COL32(120,120,120,200),
            "clic gauche: orbite  |  clic droit: pan  |  scroll: zoom");

        ImGui::End();

        // ── Render ───────────────────────────────────────────────────────
        ImGui::Render();
        int fw, fh;
        glfwGetFramebufferSize(window, &fw, &fh);
        glViewport(0, 0, fw, fh);
        glClearColor(0.07f, 0.07f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
