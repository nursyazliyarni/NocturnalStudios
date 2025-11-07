//(view/projection)
// (view/projection)
#pragma once
#include <string>

// Enable/disable per scene
void Cam2D_Enable(const std::string& scene, bool on);

// Set normalized center [0..1] per scene
void Cam2D_SetCenter(const std::string& scene, float cx, float cy);

// 1.0 = no zoom, >1 zoom in, <1 zoom out (clamped to [0.1, 5.0])
void Cam2D_SetZoom(const std::string& scene, float z);

// Convenience: set center + zoom in one call
void Cam2D_Set(const std::string& scene, float cx, float cy, float zoom);

// Relative zoom: +0.10f = +10%, -0.10f = -10%
void Cam2D_ZoomBy(const std::string& scene, float dz);

// Relative pan in normalized units (e.g., dx = ±0.02f)
void Cam2D_PanBy(const std::string& scene, float dx, float dy);

// Apply to a pixel-rect (px,py,pw,ph) for the given scene
// screenW/screenH are the framebuffer size (pixels)
void Cam2D_Apply(const std::string& scene,
    float& px, float& py, float& pw, float& ph,
    int screenW, int screenH);

// --- Camera presets & active-edit API (added) ---

struct Cam2D_Params {
    float cx = 0.5f;
    float cy = 0.5f;
    float zoom = 1.0f;
    float minZoom = 1.0f;
    float maxZoom = 3.0f;
};

// Register/overwrite a preset by scene key (scene name)
void Cam2D_RegisterPreset(const std::string& scene, const Cam2D_Params& p);

// Make a preset the active camera (copies preset -> live)
void Cam2D_ActivatePreset(const std::string& scene);

// Read/Write the currently active camera params (ImGui authoring)
Cam2D_Params Cam2D_GetActiveParams();
void Cam2D_SetActiveParams(const Cam2D_Params& p);
