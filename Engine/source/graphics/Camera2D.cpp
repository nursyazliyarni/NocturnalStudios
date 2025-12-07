//(view/projection)
// graphics/Camera2D.cpp
/*!------------------------------------------------------------------------------------------------
    File: LevelManager.h
    Project: NocturnalStudio

    This file contains ...

     Author:
         Rishika Bimal Attodi (%)
        - panning/zoom, bug fix for zoom on scene change

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "graphics/Camera2D.h"

#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <string>

namespace {
    struct CamState {
        bool  enabled = false;

        // View center in normalized scene space [0..1]
        float cx = 0.5f;
        float cy = 0.5f;

        // Zoom values
        float zoom = 1.0f;   // current (animated) zoom
        float targetZoom = 1.0f;   // where we want to go

        // Limits (1.0 = fit; never allow zooming out below fit)
        float minZoom = 1.0f;
        float maxZoom = 3.0f;

        // Smoothing factor (0.1–0.25 feels nice)
        float smooth = 0.18f;

        // added for preset/active plumbing
        float savedZoom = 1.0f;      // optional scratch

    };

    // Per-scene camera state
    std::unordered_map<std::string, CamState> g_cam;

    // which preset is currently active
    static std::string g_activeKey;

    // grab a ref to the live/active camera slot; we store it under "__active__"
    static CamState& activeCam() {
        return g_cam["__active__"];
    }

    // Visible width/height in normalized units are 1/zoom.
    // Keep the camera center so the view window stays inside [0..1].
    inline void clampCenterToContent(CamState const& s, float& cx, float& cy) {
        const float halfW = 0.5f / s.targetZoom;   // use target for stability while animating
        const float halfH = 0.5f / s.targetZoom;
        const float minCx = halfW;
        const float maxCx = 1.0f - halfW;
        const float minCy = halfH;
        const float maxCy = 1.0f - halfH;
        cx = std::clamp(cx, minCx, maxCx);
        cy = std::clamp(cy, minCy, maxCy);
    }

    inline float clampZoom(CamState const& s, float z) {
        return std::clamp(z, s.minZoom, s.maxZoom);
    }

    // Apply camera (scale around pivot) to a pixel-rect
    inline void applyOne(CamState const& c,
        float& px, float& py, float& pw, float& ph,
        int screenW, int screenH)
    {
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

// -------- Public API --------

void Cam2D_Enable(const std::string& scene, bool on) {
    g_cam[scene].enabled = on;
}

void Cam2D_SetCenter(const std::string& scene, float cx, float cy) {
    auto& s = g_cam[scene];
    cx = std::clamp(cx, 0.0f, 1.0f);
    cy = std::clamp(cy, 0.0f, 1.0f);
    clampCenterToContent(s, cx, cy);
    s.cx = cx; s.cy = cy;
}

void Cam2D_SetZoom(const std::string& scene, float z) {
    auto& s = g_cam[scene];
    s.targetZoom = clampZoom(s, z);       // drive the target
    clampCenterToContent(s, s.cx, s.cy);  // center bounds depend on zoom
}

void Cam2D_Set(const std::string& scene, float cx, float cy, float zoom) {
    Cam2D_SetCenter(scene, cx, cy);
    Cam2D_SetZoom(scene, zoom);
}

void Cam2D_ZoomBy(const std::string& scene, float dz) {
    auto& s = g_cam[scene];
    Cam2D_SetZoom(scene, s.targetZoom + dz);
}

void Cam2D_PanBy(const std::string& scene, float dx, float dy) {
    auto& s = g_cam[scene];
    float nx = std::clamp(s.cx + dx, 0.0f, 1.0f);
    float ny = std::clamp(s.cy + dy, 0.0f, 1.0f);
    clampCenterToContent(s, nx, ny);
    s.cx = nx; s.cy = ny;
}













// --- Preset registration & activation (added) ---
void Cam2D_RegisterPreset(const std::string& scene, const Cam2D_Params& p) {
    CamState& s = g_cam[scene];
    s.cx = std::clamp(p.cx, 0.0f, 1.0f);
    s.cy = std::clamp(p.cy, 0.0f, 1.0f);
    s.minZoom = p.minZoom;
    s.maxZoom = p.maxZoom;
    s.zoom = std::clamp(p.zoom, s.minZoom, s.maxZoom);
    s.targetZoom = s.zoom;
    clampCenterToContent(s, s.cx, s.cy);
}

void Cam2D_ActivatePreset(const std::string& scene) {
    auto it = g_cam.find(scene);
    if (it == g_cam.end()) return;
    g_activeKey = scene;
    activeCam() = it->second;                   // copy preset into live slot
}

Cam2D_Params Cam2D_GetActiveParams() {
    const CamState& s = activeCam();
    Cam2D_Params p;
    p.cx = s.cx; p.cy = s.cy; p.zoom = s.targetZoom; // expose target (authoring feels nicer)
    p.minZoom = s.minZoom; p.maxZoom = s.maxZoom;
    return p;
}

void Cam2D_SetActiveParams(const Cam2D_Params& p) {
    CamState& s = activeCam();
    float cx = std::clamp(p.cx, 0.0f, 1.0f);
    float cy = std::clamp(p.cy, 0.0f, 1.0f);
    s.minZoom = p.minZoom;
    s.maxZoom = p.maxZoom;
    s.targetZoom = std::clamp(p.zoom, s.minZoom, s.maxZoom);
    clampCenterToContent(s, cx, cy);
    s.cx = cx; s.cy = cy;

    // also write back to the preset (so switching scenes doesn’t blow away live edits)
    if (!g_activeKey.empty()) {
        CamState& preset = g_cam[g_activeKey];
        preset = s;
    }
}



















void Cam2D_Apply(const std::string& scene,
    float& px, float& py, float& pw, float& ph,
    int screenW, int screenH)
{
    auto it = g_cam.find(scene);
    if (it == g_cam.end()) return;

    CamState& s = it->second; // animate per frame here

    // Smoothly ease current zoom toward target
    s.zoom += (s.targetZoom - s.zoom) * s.smooth;
    if (std::fabs(s.zoom - s.targetZoom) < 1e-3f) s.zoom = s.targetZoom;

    // Make sure center stays valid as zoom changes
    clampCenterToContent(s, s.cx, s.cy);

    applyOne(s, px, py, pw, ph, screenW, screenH);
}
