//(view/projection)
#include "graphics/Camera2D.h"
// (intentionally empty – everything is inline/simple for now)

#include "graphics/Camera2D.h"
#include <unordered_map>
#include <algorithm>
#include <cmath>

namespace {
    struct CamState {
        bool  enabled = false;
        float cx = 0.5f, cy = 0.5f; // normalized pivot
        float zoom = 1.0f;          // scale factor
    };
    std::unordered_map<std::string, CamState> g_cam; // per-scene state

    inline float clampZoom(float z) {
        if (z < 0.1f) z = 0.1f;
        if (z > 5.0f) z = 5.0f;
        return z;
    }

    inline void applyOne(CamState const& c,
        float& px, float& py, float& pw, float& ph,
        int screenW, int screenH) {
        if (!c.enabled || c.zoom == 1.0f) return;

        const float pivotX = c.cx * screenW;
        const float pivotY = c.cy * screenH;

        // scale rect about pivot
        px = (px - pivotX) * c.zoom + pivotX;
        py = (py - pivotY) * c.zoom + pivotY;
        pw *= c.zoom;
        ph *= c.zoom;
    }
} // namespace

void Cam2D_Enable(const std::string& scene, bool on) {
    g_cam[scene].enabled = on;
}

void Cam2D_SetCenter(const std::string& scene, float cx, float cy) {
    auto& s = g_cam[scene];
    s.cx = std::clamp(cx, 0.0f, 1.0f);
    s.cy = std::clamp(cy, 0.0f, 1.0f);
}

void Cam2D_SetZoom(const std::string& scene, float z) {
    auto& s = g_cam[scene];
    s.zoom = clampZoom(z);
}

void Cam2D_Set(const std::string& scene, float cx, float cy, float zoom) {
    Cam2D_SetCenter(scene, cx, cy);
    Cam2D_SetZoom(scene, zoom);
}

void Cam2D_ZoomBy(const std::string& scene, float dz) {
    auto& s = g_cam[scene];
    s.zoom = clampZoom(s.zoom + dz);
}

void Cam2D_PanBy(const std::string& scene, float dx, float dy) {
    auto& s = g_cam[scene];
    s.cx = std::clamp(s.cx + dx, 0.0f, 1.0f);
    s.cy = std::clamp(s.cy + dy, 0.0f, 1.0f);
}

void Cam2D_Apply(const std::string& scene,
    float& px, float& py, float& pw, float& ph,
    int screenW, int screenH) {
    auto it = g_cam.find(scene);
    if (it == g_cam.end()) return;
    applyOne(it->second, px, py, pw, ph, screenW, screenH);
}
