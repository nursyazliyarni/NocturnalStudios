/*!------------------------------------------------------------------------------------------------
    File: Graphics.cpp
    Project: NocturnalStudio

	Brief:
		This file contain a temporarily rendering class which will be bring over
		once render class is up.
	
    Primary Author:
         Rishika Bimal Attodi (85%)
		- Created the graphics class and its function specification.
		- Implemented particle system via shaders
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added Text component rendering path in the graphics loop.
		LAYCOCK ENHUI SARAH (10%)
		- Changed it so that the loading in of textures isnt dependent on hardcoded folder
		  paths
		- Added engine asset loading (Icons, Default fonts, shaders, missing texture)

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/Graphics.h"
#include "ECS/SceneManager.h"
#include "GUI/GUI.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Input/Input.h"
#include "ResourceManager/ResourceManager.h"
#include "Timer/Time.h"
#include "LevelEditor/LevelEditor.h"
#include "Math/Math.h"
#include "Graphics/FrameBuffer.h"
#include "Timer/Fps.h"
#include "Video/VideoPlayback.h"

namespace NocturnalEngine {



	namespace
	{
		enum class SceneParticleType
		{
			None = 0,
			FloatingDots = 1,
			FallingLeaves = 2
		};

		static GLuint gSceneParticleVAO = 0;
		static GLuint gSceneParticleVBO = 0;
		static bool gSceneParticlesEnabled = false;
		static SceneParticleType gSceneParticleType = SceneParticleType::None;

		static void InitSceneParticleQuad()
		{
			if (gSceneParticleVAO != 0)
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

			glGenVertexArrays(1, &gSceneParticleVAO);
			glGenBuffers(1, &gSceneParticleVBO);

			glBindVertexArray(gSceneParticleVAO);
			glBindBuffer(GL_ARRAY_BUFFER, gSceneParticleVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		static void ShutdownSceneParticleQuad()
		{
			if (gSceneParticleVBO != 0)
			{
				glDeleteBuffers(1, &gSceneParticleVBO);
				gSceneParticleVBO = 0;
			}

			if (gSceneParticleVAO != 0)
			{
				glDeleteVertexArrays(1, &gSceneParticleVAO);
				gSceneParticleVAO = 0;
			}
		}

		static void UpdateSceneParticleSettingsFromScene()
		{
			gSceneParticlesEnabled = false;
			gSceneParticleType = SceneParticleType::None;

			const auto alive = p_ecs.GetAliveEntitiesSet();
			for (const auto& entity : alive)
			{
				if (!p_ecs.HaveComponent<SceneVFX>(entity))
					continue;

				auto& vfx = p_ecs.GetComponent<SceneVFX>(entity);

				if (!vfx.GetParticlesEnabled())
					continue;

				gSceneParticlesEnabled = true;

				switch (vfx.GetParticleType())
				{
				case SceneVFX::ParticleType::FloatingDots:
					gSceneParticleType = SceneParticleType::FloatingDots;
					break;

				case SceneVFX::ParticleType::FallingLeaves:
					gSceneParticleType = SceneParticleType::FallingLeaves;
					break;

				case SceneVFX::ParticleType::None:
				default:
					gSceneParticleType = SceneParticleType::None;
					break;
				}

				break;
			}
		}

		static void DrawSceneParticles()
		{
			if (!gSceneParticlesEnabled)
				return;

			if (gSceneParticleType == SceneParticleType::None)
				return;

			const char* shaderName = nullptr;

			switch (gSceneParticleType)
			{
			case SceneParticleType::FloatingDots:
				shaderName = "SceneParticleShader";
				break;

			case SceneParticleType::FallingLeaves:
				shaderName = "SceneLeavesShader";
				break;

			case SceneParticleType::None:
			default:
				return;
			}

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

			glBindVertexArray(gSceneParticleVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);

			if (!wasBlendEnabled)
				glDisable(GL_BLEND);

			shader->Unbind();
		}
	}








	
	/*!*************************************************************************
	Load Icon from filepath using resource manager
	****************************************************************************/
	void Graphic::LoadIconsTexture(std::string filename)
	{
		std::ifstream ifs(filename.c_str());
		std::string name, textPath;
		while (ifs >> name >> textPath)
			ResourceManager::LoadIcons(name.c_str(), (EngineAssetPath.string()+textPath).c_str());

		ifs.close();
	}
	
	/*!*************************************************************************
	Init loop for Graphics
	****************************************************************************/
	void Graphic::Init()
	{
		ResourceManager::LoadShader("QuadShader", EngineAssetPath.string() + "/Shaders/texture.shader");
		ResourceManager::LoadShader("LineShader", EngineAssetPath.string() + "/Shaders/Line.shader");
		ResourceManager::LoadShader("CircleShader", EngineAssetPath.string() + "/Shaders/Circle.shader");
		ResourceManager::LoadShader("ImpactShader", EngineAssetPath.string() + "/Shaders/Impact.shader");
		ResourceManager::LoadShader(
			"SceneParticleShader",
			EngineAssetPath.string() + "/Shaders/Fullscreen.vert.glsl",
			EngineAssetPath.string() + "/Shaders/Particle.frag.glsl"
		);

		ResourceManager::LoadShader(
			"SceneLeavesShader",
			EngineAssetPath.string() + "/Shaders/Fullscreen.vert.glsl",
			EngineAssetPath.string() + "/Shaders/Leaves.frag.glsl"
		);

		InitSceneParticleQuad();
		sMissingTextureID = ResourceManager::LoadTexture("ERROR_MISSINGTEX", EngineAssetPath.string() + "/Textures/Evil.png",true)->GetRendererID();
		
		LoadIconsTexture(EngineAssetPath.string() + "/Text/Icons.txt");
		std::string path = "Assets/Textures/";
		if (!std::filesystem::exists(path))
		{
			NOCTURNAL_ENGINE_WARN("Assets/Textures/ folder missing! Creating it...");
			std::filesystem::create_directories(path);
		}
		else
		{
			//recursive_directory_iterator goes through ALL subfolders
			for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
			{
				// Skip directories, only process files
				if (!entry.is_regular_file())
					continue;

				// Only load image files
				auto ext = entry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
					continue;

				std::string gs = entry.path().generic_string();

				// Use relative path as key to avoid name collisions
				// e.g. "Characters/horse" instead of just "horse"
				std::string relativePath = std::filesystem::relative(
					entry.path(),
					std::filesystem::path(path)
				).generic_string();

				// Remove extension from key
				std::string key = relativePath.substr(0, relativePath.find_last_of('.'));
				// key = "Characters/Cat" or "UI/Menus/Main" etc.

				ResourceManager::LoadTexture(key, gs);
				//NOCTURNAL_ENGINE_INFO("Loaded texture: {}", key);
			}
		}

		std::string fontPaths[]{ {EngineAssetPath.string() + "/Fonts" }, "Assets/Fonts","Assets/fonts" };
		for(std::string fontPath:fontPaths){
			if (std::filesystem::exists(fontPath))
			{
				for (const auto& entry : std::filesystem::directory_iterator(fontPath))
				{
					if (!entry.is_regular_file())
						continue;

					auto ext = entry.path().extension().string();
					std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
					if (ext != ".ttf" && ext != ".otf")
						continue;

					std::string fontName = entry.path().stem().string();
					if (!ResourceManager::GetFont(fontName))
					{
						ResourceManager::LoadFont(fontName, entry.path().generic_string());
						
					}
				}
			}
		}
		FrameBufferSpecification fbspec;
		fbspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::DEPTH };
		fbspec.Width = mWinData.GetWidth();
		fbspec.Height = mWinData.GetHeight();
		p_FrameBuffer->SetFrameBufferSpecification(fbspec);
		Renderer::Init();
		if (!ResourceManager::GetFont("KernlGrotesk"))
		{
			ResourceManager::LoadFont("KernlGrotesk", "Assets/Fonts/KernlGrotesk.otf");
		}
		mFont = ResourceManager::GetFont("KernlGrotesk");
		if (!mFont)
		{
			mFont = std::make_shared<Font>();
			mFont->Init();
			mFont->Load("Assets/Fonts/KernlGrotesk.otf");
		}

		for (int i = 0; i < Graphic::MAX_LAYERS; ++i)
			Graphic::layerEnabled[i] = true;

		for (int i = 0; i < Graphic::MAX_LAYERS; ++i)
			Graphic::layerDrawOrder[i] = i;

	}

	/*!*************************************************************************
	Update loop for Graphics
	****************************************************************************/
	void Graphic::Update(float frametime)
	{
		Timer::GetInstance().Start(Systems::GRAPHIC);
		Timer::GetInstance().GetDT(Systems::GRAPHIC);

		if (p_Editor->is_ShowWindow)
		{
			mcamera = &scene_camera;
		}
		else
		{
			const WindowProps windowProps = p_Editor->GetWindow()->Getter();
			glViewport(static_cast<GLint>(windowProps.m_ViewportX),
				static_cast<GLint>(windowProps.m_ViewportY),
				static_cast<GLsizei>(windowProps.m_Width),
				static_cast<GLsizei>(windowProps.m_Height));
			NocturnalEngine::Graphic::mcamera->Resize(
				static_cast<float>(windowProps.m_Width),
				static_cast<float>(windowProps.m_Height)
			);

			mcamera = &camera;
			glViewport(static_cast<GLint>(windowProps.m_ViewportX),
				static_cast<GLint>(windowProps.m_ViewportY),
				static_cast<GLsizei>(windowProps.m_Width),
				static_cast<GLsizei>(windowProps.m_Height));
			mcamera->Resize(
				static_cast<float>(windowProps.m_Width),
				static_cast<float>(windowProps.m_Height)
			);
		}

		if (FrameBufferSpecification spec = p_FrameBuffer->GetSpecification();
			p_Editor->mViewportSize.x > 0.0f && p_Editor->mViewportSize.y > 0.0f &&
			(spec.Width != p_Editor->mViewportSize.x || spec.Height != p_Editor->mViewportSize.y) &&
			p_Editor->is_ShowWindow)
		{
			p_FrameBuffer->Resize((uint32_t)p_Editor->mViewportSize.x, (uint32_t)p_Editor->mViewportSize.y);
			mcamera->Resize(p_Editor->mViewportSize.x, p_Editor->mViewportSize.y);
		}

		mRenderer->ResetInfo();
		if (p_Editor->is_ShowWindow)
		{
			p_FrameBuffer->Bind();
		}

		mRenderer->SetClearColor({ 0.0f, 0.1f, 0.1f, 1.0f });
		mRenderer->Clear();
		int value = -1;
		glClearTexImage(p_FrameBuffer->GetColorAttachmentRendererID(1), 0, GL_RGBA8, GL_INT, &value);

		UpdateSceneParticleSettingsFromScene();
		VideoPlayback::CleanupMissing(p_ecs.GetAliveEntitiesSet());
		const auto alive = p_ecs.GetAliveEntitiesSet();

		p_GUI->VPmat = mcamera->GetViewProjectionMatrix();

		auto DrawTextForLayer = [&](int layer)
		{
			for (const auto& e : alive)
			{
				if (!p_ecs.HaveComponent<Text>(e)) continue;

				auto& text = p_ecs.GetComponent<Text>(e);
				if (!text.GET_visibility()) continue;

				int textLayer = text.Get_layering_order();
				if (textLayer < 0) textLayer = 0;
				if (textLayer >= Graphic::MAX_LAYERS) textLayer = Graphic::MAX_LAYERS - 1;
				if (textLayer != layer) continue;

				glm::vec2 pos(0.0f, 0.0f);
				if (text.GetWorldSpace())
				{
					if (!p_ecs.HaveComponent<Transform>(e)) continue;
					auto& tr = p_ecs.GetComponent<Transform>(e);
					pos = glm::vec2(tr.GetWorldPos().x, tr.GetWorldPos().y);
					pos.x += text.GetPosition()[0];
					pos.y += text.GetPosition()[1];
				}
				else
				{
					pos = glm::vec2(text.GetPosition()[0], text.GetPosition()[1]);
				}

				float* c = text.GetColor();
				if (c[3] <= 0.001f) continue;

				glm::vec3 col(c[0], c[1], c[2]);

				float scale = text.GetSize() * 0.0000333f;
				if (scale < 0.0001f) scale = 0.0008f;

				std::shared_ptr<Font> font = ResourceManager::GetFont(text.GetFont());
				if (!font) font = mFont;
				if (!font) continue;

				if (text.Get_TEXT_type() == Text::text_type::single_text)
				{
					font->RenderText(text.GetContent(), pos, text.Get_line_height(), scale, *mcamera, col, text.Rot(), c[3]);
				}
			}
		};

		auto DrawSceneLayers = [&](int startOrderIndex, int endOrderIndex)
		{
			for (int orderIndex = startOrderIndex; orderIndex <= endOrderIndex; ++orderIndex)
			{
				int layer = Graphic::layerDrawOrder[orderIndex];
				if (!Graphic::layerEnabled[layer]) continue;

				mRenderer->Begin(*mcamera);

				for (auto i = mEntities.begin(); i != mEntities.end(); ++i)
				{
					auto& transform = p_ecs.GetComponent<Transform>(*i);
					auto& sprite = p_ecs.GetComponent<Sprite>(*i);
					int layerId = sprite.LayerOrder;
					if (layerId < 0) layerId = 0;
					if (layerId >= Graphic::MAX_LAYERS) layerId = Graphic::MAX_LAYERS - 1;
					if (layerId != layer) continue;

					std::shared_ptr<Texture> renderTexture = GETTEXTURE(sprite.GetTexture());
					if (p_ecs.HaveComponent<Video>(*i))
					{
						auto& video = p_ecs.GetComponent<Video>(*i);
						auto videoTexture = VideoPlayback::UpdateAndGetTexture(*i, video, frametime, p_Editor->is_ShowWindow);
						if (videoTexture)
						{
							renderTexture = videoTexture;
						}
						else if (video.hideWhenFinished && !video.loop)
						{
							continue;
						}
					}

					if (sprite.is_Animated && sprite.Get_SPRITE_type() == Sprite::sprite_type::texture)
					{
						mAimator.UpdateAnimation(frametime, sprite);
					}

					if (sprite.is_SpriteSheet)
					{
						mIndex1 = SpriteRender::CreateSprite(
							renderTexture,
							{ sprite.GetIndex().x, sprite.GetIndex().y },
							{ sprite.GetUVCoor().x, sprite.GetUVCoor().y }
						);

						mRenderer->DrawSprite(
							{ transform.GetWorldPos().x, transform.GetWorldPos().y },
							{ transform.GetScale().x, transform.GetScale().y },
							transform.GetRot(),
							mIndex1
						);
					}

					else if (sprite.Get_SPRITE_type() == Sprite::sprite_type::texture)
					{
						mRenderer->DrawQuad(
							{ transform.GetWorldPos().x, transform.GetWorldPos().y },
							{ transform.GetScale().x, transform.GetScale().y },
							transform.GetRot(),
							renderTexture
						);
					}

					else if (sprite.Get_SHAPE_type() == Sprite::shape_type::rectangle)
					{
						if (sprite.is_Animated) {
							if (p_ecs.HaveComponent<Text>(*i)) {
								auto& text{ p_ecs.GetComponent<Text>(*i) };
								if (text.GET_visibility()) {
									mAimator.UpdateShapeAnimation(frametime, sprite);
									mRenderer->DrawDashedRect(
										{ transform.GetPos().x, transform.GetPos().y, 0.0f },
										{ transform.GetScale().x, transform.GetScale().y },
										{ sprite.rgba_color[0], sprite.rgba_color[1], sprite.rgba_color[2], sprite.rgba_color[3] },
										sprite.offset, sprite.line_len, sprite.gap_len
									);
								}
							}
							
							else {
								mAimator.UpdateShapeAnimation(frametime, sprite);
								mRenderer->DrawDashedRect(
									{ transform.GetPos().x, transform.GetPos().y, 0.0f },
									{ transform.GetScale().x, transform.GetScale().y },
									{ sprite.rgba_color[0], sprite.rgba_color[1], sprite.rgba_color[2], sprite.rgba_color[3] },
									sprite.offset, sprite.line_len, sprite.gap_len
								);
							}
						}

						else { //it is not animated shape
							mRenderer->DrawRect(
								{ transform.GetPos().x, transform.GetPos().y, 0.0f },
								{ transform.GetScale().x, transform.GetScale().y },
								{ sprite.rgba_color[0], sprite.rgba_color[1], sprite.rgba_color[2], sprite.rgba_color[3] }
							);
						}
					}

					else if (sprite.Get_SHAPE_type() == Sprite::shape_type::circle)
					{
						NocturnalEngine::Matrix4x4 translate = NocturnalEngine::Translate4x4(
							translate, transform.GetWorldPos().x, transform.GetWorldPos().y, 0.0f
						);
						NocturnalEngine::Matrix4x4 scale = NocturnalEngine::Scale4x4(
							scale, sprite.circle_radius * 2, sprite.circle_radius * 2, sprite.circle_radius * 2
						);
						NocturnalEngine::Matrix4x4 Transform = translate * scale;
						mRenderer->DrawCircle(
							Transform,
							{ sprite.rgba_color[0], sprite.rgba_color[1], sprite.rgba_color[2], sprite.rgba_color[3] },
							0.01f
						);
					}

					if (p_Editor->mDebugDraw &&
						p_ecs.HaveComponent<Collider>(*i) &&
						(p_ecs.GetComponent<Collider>(*i)[0].is_Alive || p_ecs.GetComponent<Collider>(*i)[1].is_Alive))
					{
						for (int u = 0; u < 2; ++u)
						{
							auto& collider = p_ecs.GetComponent<Collider>(*i);
							if (collider[u].mCol == Collider::ColliderType::rect || collider[u].mCol == Collider::ColliderType::box)
							{
								mRenderer->DrawRect(
									{
										transform.GetWorldPos().x + collider[u].mOffset.x + (collider[u].mMax.x / 2) - (collider[u].mMin.x / 2),
										transform.GetWorldPos().y + collider[u].mOffset.y + (collider[u].mMax.y / 2) - (collider[u].mMin.y / 2),
										0.0f
									},
									{ collider[u].mMin.x + collider[u].mMax.x, collider[u].mMin.y + collider[u].mMax.y },
									{ 1.0f, 0.0f, 0.0f, 1.0f }
								);
							}
							if (collider[u].mCol == Collider::ColliderType::circle ||
								collider[u].mCol == Collider::ColliderType::bubble ||
								collider[u].mCol == Collider::ColliderType::bossball)
							{
								NocturnalEngine::Matrix4x4 translate = NocturnalEngine::Translate4x4(
									translate,
									transform.GetWorldPos().x + collider[u].mOffset.x,
									transform.GetWorldPos().y + collider[u].mOffset.y,
									0.0f
								);
								NocturnalEngine::Matrix4x4 scale = NocturnalEngine::Scale4x4(
									scale, collider[u].mRadius * 2, collider[u].mRadius * 2, collider[u].mRadius * 2
								);
								NocturnalEngine::Matrix4x4 Transform = translate * scale;
								mRenderer->DrawCircle(Transform, { 0.5f, 0.4f, 1.0f, 1.0f }, 0.01f);
							}
						}
					}
				}

				mRenderer->End();

				DrawTextForLayer(layer);
			}
		};

		DrawSceneLayers(0, 0);

		if (gSceneParticlesEnabled)
		{
			DrawSceneParticles();
		}

		DrawSceneLayers(1, Graphic::MAX_LAYERS - 1);

		for (auto const& entity : mEntities)
		{
			if (p_ecs.HaveComponent<NameTag>(entity) && p_ecs.GetComponent<NameTag>(entity).GetNameTag() == "Splash Screen")
			{
				p_ecs.GetComponent<Transform>(entity).GetScale().x += frametime;
				p_ecs.GetComponent<Transform>(entity).GetScale().y += frametime;
			}
		}

		if (p_GUI->check_pause() == true && p_GUI->Check_menu() == false)
		{
			mRenderer->Begin(*mcamera);
			mRenderer->DrawQuad({ camera.GetPosition().x, camera.GetPosition().y }, { 3.52f, 1.89f }, 0.0f, GETTEXTURE("EndGameUI"));
			mRenderer->DrawQuad({ camera.GetPosition().x + -0.89f, camera.GetPosition().y + -0.01f }, { 1.21f, 1.54f }, 0.0f, GETTEXTURE("Avatar"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.522f, camera.GetPosition().y + 0.118f }, { 1.125f, 1.222f }, 0.0f, GETTEXTURE("MenuPanel"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + 0.361f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + 0.186f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + -0.007f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + -0.198f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + -0.556f }, { 1.350f, 0.175f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->End();
		}

		const float sceneTransitionAlpha = p_Scene->GetSceneTransitionAlpha();
		if (sceneTransitionAlpha > 0.0f)
		{
			mRenderer->Begin(*mcamera);
			// Draw a simple fullscreen black overlay to visualize the scene transition fade.
			mRenderer->DrawQuad(
				{ mcamera->GetPosition().x, mcamera->GetPosition().y },
				{ 20.0f, 20.0f },
				0.0f,
				{ 0.0f, 0.0f, 0.0f, sceneTransitionAlpha }
			);
			mRenderer->End();
		}

		p_FrameBuffer->UnBind();

		mcamera->MouseScrolling();
		if (p_Input->KeyPressed(GLFW_KEY_F2) && !p_Editor->is_ShowWindow)
		{
			misFps = !misFps;
		}
		if (misFps)
		{
			auto i = std::to_string(FramePerSec::GetInstance().GetFps());
			mFont->RenderText("FPS: " + i, { camera.GetPosition().x + 0.2f, camera.GetPosition().y + 0.2f }, 0,
				0.0008f, camera, { 0.87f, 0.92f, 0.18f });
		}

		Timer::GetInstance().Update(Systems::GRAPHIC);
	}

	/*!*************************************************************************
	End loop for Graphics
	****************************************************************************/
	void Graphic::End()
	{
		ShutdownSceneParticleQuad();
		ResourceManager::clear();
		Renderer::ShutDown();
	}

}
