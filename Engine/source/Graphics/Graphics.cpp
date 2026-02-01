/*!------------------------------------------------------------------------------------------------
    File: Graphics.cpp
    Project: NocturnalStudio

	Brief:
		This file contain a temporarily rendering class which will be bring over
		once render class is up.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the graphics class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/Graphics.h"
#include "GUI/GUI.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Input/Input.h"
#include "ResourceManager/ResourceManager.h"
#include "Timer/Time.h"
#include "LevelEditor/LevelEditor.h"
#include "Math/Math.h"
#include "Graphics/FrameBuffer.h"
#include "Timer/Fps.h"
#include "ECS/SceneManager.h"

namespace NocturnalEngine {
	/*!*************************************************************************
	Load Icon from filepath using resource manager
	****************************************************************************/
	void Graphic::LoadIconsTexture(std::string filename)
	{
		std::ifstream ifs(filename.c_str());
		std::string name, textPath;
		while (ifs >> name >> textPath)
			ResourceManager::LoadIcons(name.c_str(), textPath.c_str());

		ifs.close();
	}
	
	/*!*************************************************************************
	Init loop for Graphics
	****************************************************************************/
	void Graphic::Init()
	{
		
		ResourceManager::LoadShader("QuadShader", "Assets/Shaders/texture.shader");
		ResourceManager::LoadShader("LineShader", "Assets/Shaders/Line.shader");
		ResourceManager::LoadShader("CircleShader", "Assets/Shaders/Circle.shader");
		ResourceManager::LoadShader("ImpactShader", "Assets/Shaders/Impact.shader");
		
		LoadIconsTexture("Assets/Text/Icons.txt");
		std::string path = {"Assets/Textures/"};
		std::string folderpath[] = {"Characters", "Environment", "UI", "CutScene", "Dialogues", "Play"};
		
		for (const auto& i : folderpath)
		{
			std::filesystem::path currentPath = path + i;
			auto currentFolders = std::filesystem::directory_iterator(currentPath);
			for (const auto& entry : currentFolders)
			{
				std::string gs = entry.path().generic_string();
				std::size_t pos = gs.find_last_of("/");
				std::size_t pos1 = gs.find(".");
				std::string Imagename = gs.substr(pos + 1, pos1 - pos - 1);
				ResourceManager::LoadTexture(Imagename, gs);
			}
		}
		std::string fontPath = "Assets/fonts";
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
		FrameBufferSpecification fbspec;
		fbspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::DEPTH };
		fbspec.Width = mWinData.GetWidth();
		fbspec.Height = mWinData.GetHeight();
		p_FrameBuffer->SetFrameBufferSpecification(fbspec);
		Renderer::Init();
		if (!ResourceManager::GetFont("KernlGrotesk"))
		{
			ResourceManager::LoadFont("KernlGrotesk", "Assets/fonts/KernlGrotesk.otf");
		}
		mFont = ResourceManager::GetFont("KernlGrotesk");
		if (!mFont)
		{
			mFont = std::make_shared<Font>();
			mFont->Init();
			mFont->Load("Assets/fonts/KernlGrotesk.otf");
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
			mcamera = &scene_camera;
		else
		{
			glViewport(0, 0, p_Editor->GetWindow()->Getter().m_Width, p_Editor->GetWindow()->Getter().m_Height);
			NocturnalEngine::Graphic::mcamera->Resize(static_cast<float>(p_Editor->GetWindow()->Getter().m_Width), static_cast<float>(p_Editor->GetWindow()->Getter().m_Height));

			mcamera = &camera;
			if (!p_Editor->is_ShowWindow)
			{
				glViewport(0, 0, p_Editor->GetWindow()->Getter().m_Width, p_Editor->GetWindow()->Getter().m_Height);
				mcamera->Resize(static_cast<float>(p_Editor->GetWindow()->Getter().m_Width), static_cast<float>(p_Editor->GetWindow()->Getter().m_Height));
			}
		}
		//Resize
		if (FrameBufferSpecification spec = p_FrameBuffer->GetSpecification();
			p_Editor->mViewportSize.x > 0.0f && p_Editor->mViewportSize.y > 0.0f && // zero sized framebuffer is invalid
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
	
		mRenderer->Begin(*mcamera);// begin of the renderer 
		//mFont->RenderText("TEXT TEST", { 0.f, 0.f }, 0.001f, *mcamera, { 1,1,1 }); // for testing

		p_GUI->VPmat = mcamera->GetViewProjectionMatrix();
		//std::map<std::string, double>animal_attribute_map;

		for (int orderIndex = 0; orderIndex < Graphic::MAX_LAYERS; ++orderIndex)
		{
			int layer = Graphic::layerDrawOrder[orderIndex];
			if (!Graphic::layerEnabled[layer]) continue;

			for (auto i = mEntities.begin(); i != mEntities.end(); i++)
			{
				
				
				auto& transform = p_ecs.GetComponent<Transform>(*i);
				auto& sprite = p_ecs.GetComponent<Sprite>(*i);
				int layerId = sprite.LayerOrder;
				if (layerId < 0) layerId = 0;
				if (layerId >= Graphic::MAX_LAYERS) layerId = Graphic::MAX_LAYERS - 1;

				if (layerId == layer)
				{//render the lowest layer level then follow on
					if (sprite.is_Animated)
					{
						mAimator.UpdateAnimation(frametime,  p_ecs.GetComponent<Sprite>(*i));
					}

					if (sprite.is_SpriteSheet)
					{

						mIndex1 = SpriteRender::CreateSprite(GETTEXTURE(sprite.GetTexture()), { sprite.GetIndex().x, sprite.GetIndex().y },
							{ sprite.GetUVCoor().x, sprite.GetUVCoor().y });

						mRenderer->DrawSprite({ transform.GetPos().x , transform.GetPos().y }, { transform.GetScale().x , transform.GetScale().y },
							transform.GetRot(), mIndex1);
					}

					else
					{
						mRenderer->DrawQuad({ transform.GetPos().x, transform.GetPos().y }, { transform.GetScale().x, transform.GetScale().y },
							transform.GetRot(), GETTEXTURE(sprite.GetTexture()));
					}

					if (p_Editor->mDebugDraw)
					{
						if (p_ecs.HaveComponent<Collider>(*i) && ((p_ecs.GetComponent<Collider>(*i)[0].is_Alive) || (p_ecs.GetComponent<Collider>(*i)[1].is_Alive))) {
							for (int u = 0; u < 2; u++) {
								if (p_ecs.GetComponent<Collider>(*i)[u].mCol == Collider::ColliderType::rect || p_ecs.GetComponent<Collider>(*i)[u].mCol == Collider::ColliderType::box)
								{
									auto& collider = p_ecs.GetComponent<Collider>(*i);
									mRenderer->DrawRect({ transform.GetPos().x + collider[u].mOffset.x + (collider[u].mMax.x / 2) - (collider[u].mMin.x / 2) , transform.GetPos().y + collider[u].mOffset.y + (collider[u].mMax.y / 2) - (collider[u].mMin.y / 2), 0.0f },
										{ collider[u].mMin.x + collider[u].mMax.x , collider[u].mMin.y + collider[u].mMax.y },
										{ 1.0f, 0.0f, 0.0f,1.0f });
								}
								if (p_ecs.GetComponent<Collider>(*i)[u].mCol == Collider::ColliderType::circle || p_ecs.GetComponent<Collider>(*i)[u].mCol == Collider::ColliderType::bubble
									|| p_ecs.GetComponent<Collider>(*i)[u].mCol == Collider::ColliderType::bossball) {
									auto& collider = p_ecs.GetComponent<Collider>(*i);
									NocturnalEngine::Matrix4x4 translate = NocturnalEngine::Translate4x4(translate, transform.GetPos().x + collider[u].mOffset.x, transform.GetPos().y + collider[u].mOffset.y, 0.0f);
									NocturnalEngine::Matrix4x4 scale = NocturnalEngine::Scale4x4(scale, collider[u].mRadius * 2, collider[u].mRadius * 2, collider[u].mRadius * 2);
									NocturnalEngine::Matrix4x4 Transform = translate * scale;
									mRenderer->DrawCircle(Transform, { 0.5f,0.4f,1.0f, 1.0f }, 0.01f);
								}
							}
						}
					}
				}
			}
		}

		
		for (auto const& entity : mEntities)
		{
			if (p_ecs.HaveComponent<NameTag>(entity) && !p_Editor->is_ShowWindow)
			{

			}
			if (p_ecs.HaveComponent<NameTag>(entity) && p_ecs.GetComponent<NameTag>(entity).GetNameTag() == "Splash Screen")
			{
				p_ecs.GetComponent<Transform>(entity).GetScale().x += frametime;
				p_ecs.GetComponent<Transform>(entity).GetScale().y += frametime;
				
			}

		}

		

		if (p_GUI->check_pause() == true && p_GUI->Check_menu() == false)
		{
			//UI background
			mRenderer->DrawQuad({ camera.GetPosition().x, camera.GetPosition().y}, { 3.52f, 1.89f }, 0.0f, GETTEXTURE("EndGameUI"));
			mRenderer->DrawQuad({ camera.GetPosition().x + -0.89f, camera.GetPosition().y + -0.01f }, { 1.21f, 1.54f }, 0.0f, GETTEXTURE("Avatar"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.522f, camera.GetPosition().y + 0.118f }, { 1.125f, 1.222f }, 0.0f, GETTEXTURE("MenuPanel"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f,camera.GetPosition().y + 0.361f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + 0.186f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + -0.007f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + -0.198f }, { 0.802f, 0.123f }, 0.0f, GETTEXTURE("ResumeButton"));
			mRenderer->DrawQuad({ camera.GetPosition().x + 0.526f, camera.GetPosition().y + -0.556f }, { 1.350f, 0.175f }, 0.0f, GETTEXTURE("ResumeButton"));

		}
		mRenderer->End();

		const auto alive = p_ecs.GetAliveEntitiesSet();
		for (const auto& e : alive)
		{
			if (!p_ecs.HaveComponent<Text>(e)) continue;
			auto& text = p_ecs.GetComponent<Text>(e);

			glm::vec2 pos(0.0f, 0.0f);

			if (text.GetWorldSpace())
			{
				if (!p_ecs.HaveComponent<Transform>(e)) continue;
				auto& tr = p_ecs.GetComponent<Transform>(e);

				pos = glm::vec2(tr.GetPos().x, tr.GetPos().y);
				pos.x += text.GetPosition()[0];
				pos.y += text.GetPosition()[1];
			}
			else
			{
				pos = glm::vec2(text.GetPosition()[0], text.GetPosition()[1]);
			}

			float* c = text.GetColor();
			glm::vec3 col(c[0], c[1], c[2]);

			float scale = text.GetSize() * 0.0000333f;
			if (scale < 0.0001f) scale = 0.0008f;

			std::shared_ptr<Font> font = ResourceManager::GetFont(text.GetFont());
			if (!font)
				font = mFont;
			if (font)
				font->RenderText(text.GetContent(), pos, scale, *mcamera, col);
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
			mFont->RenderText("FPS: " + i, { camera.GetPosition().x + 0.2f, camera.GetPosition().y + 0.2f },
				0.0008f, camera, { 0.87f, 0.92f, 0.18f });
		}
		Timer::GetInstance().Update(Systems::GRAPHIC);
		
		
	}

	/*!*************************************************************************
	End loop for Graphics
	****************************************************************************/
	void Graphic::End()
	{
		ResourceManager::clear();
		Renderer::ShutDown();
	}

}
