/*!------------------------------------------------------------------------------------------------
    File: SceneParticleRenderer.h
    Project: NocturnalStudio

    Brief:
        Renderer for scene-wide fullscreen particle / VFX passes.
        This is intended for reusable scene effects such as floating dots,
        dust, glow motes, etc., controlled externally by scene data.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <GL/glew.h>

namespace NocturnalEngine
{
    class SceneParticleRenderer
    {
    public:
        enum class ParticleType
        {
            None = 0,
            FloatingDots = 1
        };

        SceneParticleRenderer() = default;
        ~SceneParticleRenderer() = default;

        void Init();
        void Draw(ParticleType type);
        void Shutdown();

    private:
        void InitFullscreenQuad();
        const char* GetShaderName(ParticleType type) const;

    private:
        GLuint mQuadVAO = 0;
        GLuint mQuadVBO = 0;
        bool mInitialized = false;
    };
}