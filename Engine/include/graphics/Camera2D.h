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
