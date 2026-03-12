/*!------------------------------------------------------------------------------------------------
    File: SceneParticleRenderer.cpp
    Project: NocturnalStudio

    Brief:
        Implementation of a reusable fullscreen scene particle / VFX renderer.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/SceneParticleRenderer.h"
#include "Graphics/Shader.h"
#include "ResourceManager/ResourceManager.h"

namespace NocturnalEngine
{
    void SceneParticleRenderer::Init()
    {
        if (mInitialized)
            return;

        InitFullscreenQuad();
        mInitialized = true;
    }

    void SceneParticleRenderer::InitFullscreenQuad()
    {
        if (mQuadVAO != 0)
            return;

        float quadVertices[] =
        {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,

            -1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f
        };

        glGenVertexArrays(1, &mQuadVAO);
        glGenBuffers(1, &mQuadVBO);

        glBindVertexArray(mQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    const char* SceneParticleRenderer::GetShaderName(ParticleType type) const
    {
        switch (type)
        {
        case ParticleType::FloatingDots:
            return "SceneParticleShader";

        case ParticleType::FallingLeaves:
            return "SceneLeavesShader";

        case ParticleType::None:
        default:
            return nullptr;
        }
    }

    void SceneParticleRenderer::Draw(ParticleType type)
    {
        if (!mInitialized)
            Init();

        if (type == ParticleType::None)
            return;

        const char* shaderName = GetShaderName(type);
        if (!shaderName)
            return;

        auto shader = ResourceManager::GetShader(shaderName);
        if (!shader)
            return;

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        const float width = static_cast<float>(viewport[2]);
        const float height = static_cast<float>(viewport[3]);

        shader->Bind();
        shader->SetUniform("uResolution", glm::vec2(width, height));
        shader->SetUniform("uTime", static_cast<float>(glfwGetTime()));

        GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(mQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        if (!wasBlendEnabled)
            glDisable(GL_BLEND);

        shader->Unbind();
    }

    void SceneParticleRenderer::Shutdown()
    {
        if (mQuadVBO != 0)
        {
            glDeleteBuffers(1, &mQuadVBO);
            mQuadVBO = 0;
        }

        if (mQuadVAO != 0)
        {
            glDeleteVertexArrays(1, &mQuadVAO);
            mQuadVAO = 0;
        }

        mInitialized = false;
    }
}