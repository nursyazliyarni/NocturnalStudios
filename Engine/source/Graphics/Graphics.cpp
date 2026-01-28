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
		FrameBufferSpecification fbspec;
		fbspec.Attachments = { FrameBufferTextureFormat::RGBA8, FrameBufferTextureFormat::RED_INTEGER, FrameBufferTextureFormat::DEPTH };
		fbspec.Width = mWinData.GetWidth();
		fbspec.Height = mWinData.GetHeight();
		p_FrameBuffer->SetFrameBufferSpecification(fbspec);
		Renderer::Init();
		mFont->Init();
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
	

		p_GUI->VPmat = mcamera->GetViewProjectionMatrix();
		

		for (auto layer = 0; layer < 8; layer++)
		{
			for (auto i = mEntities.begin(); i != mEntities.end(); i++)
			{
				
				auto& transform = p_ecs.GetComponent<Transform>(*i);
				auto& sprite = p_ecs.GetComponent<Sprite>(*i);
				if (sprite.LayerOrder == layer)
				{//render the lowest layer level then follow on
					if (sprite.is_Animated)
					{
						mAimator.UpdateAnimation(frametime, p_ecs.GetComponent<Sprite>(*i));
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

					if (p_ecs.HaveComponent<ItemsAttributes>(*i) && p_ecs.GetComponent<ItemsAttributes>(*i).is_dragging) {
						mRenderer->DrawQuad({ p_ecs.GetComponent<ItemsAttributes>(*i).duplicate_item_pos }, { transform.GetScale().x, transform.GetScale().y },
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