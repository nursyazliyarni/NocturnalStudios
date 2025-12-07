/*!------------------------------------------------------------------------------------------------
    File: DebugDraw.h
    Project: NocturnalStudio

    This file contains the declarations for namespace debug that holds the shapes needed for 
    debugging

    Primary Author:
         Nur Syazliyarni (100%)

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once 
#include "../math/Matrix.h"
#include "../math/Vector.h"
#include <vector>

namespace Debug
{
    struct Line
    {
        Vector2D start;
        Vector2D end;
        unsigned int color;
    };

    struct Circle
    {
        Vector2D center;
        float radius;
        unsigned int color;
        int segments;
    };

    struct Rect
    {
        Vector2D pos;
        float w, h;
        unsigned int color;
    };

    /*
    struct DebugSprite
    {
        GLuint textureID;
        Vector2D pos;
        Vector2D size;
        float rotation = 0.0f;
        unsigned int color = 0xFFFFFFFF; // RGBA tint
    };

    
    struct DebugImage {
        GLuint textureID;   // OpenGL texture handle
        Vector2D pos;       // bottom-left
        Vector2D size;      // width, height
        unsigned int color; // tint 
    };

    struct DebugPrefab {
        std::string name;  // prefab name 
        Vector2D pos;      // world position
        float scale;       // scale factor
        float rotation;    // optional rotation
    };
    */

    class Drawer
    {
    public:
        static Drawer& Instance();

        void AddRect(float x, float y, float w, float h, unsigned int color);
        void AddCircle(float cx, float cy, float r, unsigned int color, int segments = 32); // default 32 segments
        void AddLine(float x1, float y1, float x2, float y2, unsigned int color);
        

        /*
        void AddSprite(GLuint tex, float x, float y, float w, float h, float rotation = 0.0f, unsigned int color = 0xFFFFFFFF);
        void AddImage(GLuint tex, float x, float y, float w, float h, unsigned int color = 0xFFFFFFFF);
        void AddPrefab(const std::string& prefabName, const Vector2D& pos, float scale = 1.0f, float rotation = 0.0f);
        */

        void Render(const Matrix<3>& cameraMatrix);
        void Clear();
        void Toggle(bool active);
        bool IsActive() const;

    private:
        Drawer() = default;
        bool active = false;
        std::vector<Rect> rects;
        std::vector<Circle> circles;
        std::vector<Line> lines;
        
        /*
        std::vector<DebugSprite> sprites;
        std::vector<DebugImage> images;
        std::vector<DebugPrefab> prefabs;
        */
    };
}
