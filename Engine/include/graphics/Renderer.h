// OpenGL draw API (clear, drawSprite, debug lines)

#pragma once
// super–simple immediate-mode 2D renderer (compat profile)


class Renderer {
public:
    // Setup a pixel-space 2D projection (origin = top-left).
    void init2D(int w, int h);
    // Update projection/viewport after window resize.
    void resize(int w, int h);
    // Clear background.
    void clear(float r, float g, float b);

    // Visible content
    void tri(float x1, float y1, float x2, float y2, float x3, float y3);     // filled triangle
    void rect(float x, float y, float w, float h);                           // filled rectangle

    // Debug-draw (for the rubric)
    void line(float x1, float y1, float x2, float y2);
    void rectOutline(float x, float y, float w, float h);
    void circle(float cx, float cy, float radius, int segments = 32);
};