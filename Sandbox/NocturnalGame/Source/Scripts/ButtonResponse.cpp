/*!------------------------------------------------------------------------------------------------
    File: ButtonResponse.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the ButtonResponse script class which is responsible 
        for managing button responses within the engine.
	
    Primary Author:
         LAU XIANG YANG (99%)
		- Created the ButtonResponse class and its function specification.
	Seconday Author(s):
		LAYCOCK ENHUI SARAH (1%)
		- Added random sound functionality for specific ingame buttons so 
		  that they play a random sound instead of one singular sound
		- updated the collision logic so it will actually work with the colliders
		  and and accept circle colliders

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/ButtonResponse.h"
#include "ECS/SceneManager.h"
#include "GUI/GUI.h"
#include "Graphics/Graphics.h"
#include "Input/Input.h"
#include "Scripts/GlobalGameManager.h"
#include "Math/Random.h"
#include "Scripts/TabButton.h"


namespace NocturnalEngine
{

	void ButtonResponse::Start() {}

	void ButtonResponse::Update(float Frametime)
	{

		Graphic::camera.SetPosition({ 0.0f,0.0f,0.0f });// ensure that camera is set to  center at the start
		UNREFERENCED_PARAMETER(Frametime);
		//this set of codes are to allow easy access to the game component when coding
		auto& transform = p_ecs.GetComponent<Transform>(GetScriptEntityID());
		auto& ID_tag = p_ecs.GetComponent<Tag>(GetScriptEntityID());
		auto& col = p_ecs.GetComponent<Collider>(GetScriptEntityID());
		auto& spt = p_ecs.GetComponent<Sprite>(GetScriptEntityID());


		// ------------------------------------------------------------
// Keyboard shortcuts for scene switching (works anywhere)
// 1 / W -> Flamingo
// 2 / A -> Seal
// 3 / S -> Horse
// 4 / D -> Owl
// ------------------------------------------------------------
		if (p_GUI->Check_HTP() == false) // don't allow shortcuts during HTP overlay
		{
			if (p_Input->KeyPressed(GLFW_KEY_1) || p_Input->KeyPressed(GLFW_KEY_W))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");
				return;
			}
			if (p_Input->KeyPressed(GLFW_KEY_2) || p_Input->KeyPressed(GLFW_KEY_A))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/SealScene.json");
				return;
			}
			if (p_Input->KeyPressed(GLFW_KEY_3) || p_Input->KeyPressed(GLFW_KEY_S))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/HorseScene.json");
				return;
			}
			if (p_Input->KeyPressed(GLFW_KEY_4) || p_Input->KeyPressed(GLFW_KEY_D))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
				p_Scene->setSceneToLoad("Assets/Scene/OwlScene.json");
				return;
			}
		}



		//if (spt.GetTexture() == "HowToPlay" && p_Input->KeyPressed(GLFW_KEY_ESCAPE)) //if how to play screen in on and escape is pressed, return screen back to button mode
		//{
		//	spt.SetTexture("UI/htp_button_Idle");
		//	transform.SetPos({ 0.0f,-0.25f });
		//	transform.SetScale(1.0f, 0.2f);
		//	p_GUI->toggle_HTP(); // toggle HTP off 

		//}

		if (p_GUI->Check_HTP() == false) // if HTP mode is off allow button to be click and interacted with
		{
			//this code is to check the current state of the mouse
			if (curr_state != button_state::click)
				curr_state = button_state::idle;// return to idle whe the state is set to release at the start of the next game loop


			if (p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT) && curr_state == button_state::idle)
			{
				curr_state = button_state::click; //if left mouse is held down set mouse state to click
			}

			else if (p_Input->MouseIsReleased(GLFW_MOUSE_BUTTON_LEFT) && curr_state == button_state::click)
			{
				curr_state = button_state::release;//when mouse key is release set state to release
			}
		}


		if (is_within_box(p_GUI->MousePosition, col, transform))//check if the system is hovering over an object, change to the hover sprite if it is
		{
			if (initial_hover && p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > 0) && (p_GUI->Check_HTP() == false) && (p_Scene->isGameplay == false))
			{
				p_ecs.GetComponent<Audio>(GetScriptEntityID())[1].should_play = true; //menu hover audio
				initial_hover = false;
				leftcollider =true;
				entercollider = true;
			}

			if (ID_tag.GetTag() == "Start" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				//spt.SetTexture("UI/Start_Hover");
				if (entercollider)
				{
					transform.ScaleBy(1.2f);
					entercollider = false;
				}
			}

			if (ID_tag.GetTag() == "Quit" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				//spt.SetTexture("UI/Quit_Hover");
				if (entercollider)
				{
					transform.ScaleBy(1.2f);
					entercollider = false;
				}
			}

			if (ID_tag.GetTag() == "Options" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				//spt.SetTexture("UI/Options_Hover");
				if (entercollider)
				{
					transform.ScaleBy(1.2f);
					entercollider = false;
				}
			}

			if (ID_tag.GetTag() == "HowToPlay" && spt.GetTexture() != "HowToPlay" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				//spt.SetTexture("UI/htp_button_Hover");
				if (entercollider)
				{
					transform.ScaleBy(1.2f);
					entercollider = false;
				}
			}

			if (ID_tag.GetTag() == "MainMenu" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/MM_B");
			}

			if (ID_tag.GetTag() == "Restart" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/RESTART_B");
			}
			if (ID_tag.GetTag() == "RestartT" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/Xmark");
			}
			if (ID_tag.GetTag() == "ProceedT" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/Xmark");
			}
			if (ID_tag.GetTag() == "QuitT" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/Xmark");
			}

			if (ID_tag.GetTag() == "Resume" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/Resume_Hover");
			}

			if (ID_tag.GetTag() == "PRestart" && !(p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT)))
			{
				spt.SetTexture("UI/Restart_Hover");
			}

			if (ID_tag.GetTag() == "Back")
			{
				spt.SetTexture("UI/BACK_B");//to be change
			}

			if (ID_tag.GetTag() == "No")
			{
				//spt.SetTexture("UI/BlankArrow");//to be change

				if (entercollider)
				{
					transform.ScaleBy(1.2f);
					entercollider = false;
				}
			}

			if (ID_tag.GetTag() == "Yes")
			{
				//spt.SetTexture("UI/BlankArrow");//to be change

				if (entercollider)
				{
					transform.ScaleBy(1.2f);
					entercollider = false;
				}
			}

			if (curr_state == button_state::release)// when mouse button is release, call the button functions, release is selected to prevent function from calling when mouse is held down 
			{
				if (ID_tag.GetTag() == "Start" && p_GUI->Check_HTP() == false)
				{

					Graphic::mcamera->SetZoomLevel(1.0f);
					p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");
				}

				if (ID_tag.GetTag() == "Restart")
				{
					p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");//reset the game level in game over
				}
				if (ID_tag.GetTag() == "RestartT")
				{
					p_GameManager.ResetStats();
					
					p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");//reset the game level in game over
				}
				if (ID_tag.GetTag() == "ProceedT")
				{
					p_GameManager.ResetStats();

					p_Scene->setSceneToLoad("Assets/Scene/Menu.json");//reset the game level in game over
				}

				if (ID_tag.GetTag() == "Quit" && p_GUI->Check_HTP() == false)
				{
					p_Scene->setSceneToLoad("Assets/Scene/Quit.json");//exit to the are you sure page
				}
				if (ID_tag.GetTag() == "QuitT" && p_GUI->Check_HTP() == false)
				{
					p_GameManager.ResetStats();
					p_Scene->setSceneToLoad("Assets/Scene/Quit.json");//exit to the are you sure page
				}

				if (ID_tag.GetTag() == "HowToPlay" && p_GUI->Check_HTP() == false) // convert the button into a how to play screen
				{
					p_Scene->setSceneToLoad("Assets/Scene/HowToPlay.json");// escape to how to play scene
				}

				if (ID_tag.GetTag() == "Options" && p_GUI->Check_HTP() == false)
				{
					p_Scene->setSceneToLoad("Assets/Scene/Option.json");// escape to  the option menu
				}

				if (ID_tag.GetTag() == "MainMenu" && p_GUI->Check_HTP() == false)
				{
					p_Scene->setSceneToLoad("Assets/Scene/Menu.json");//return to main menu from game over
				}
				if (ID_tag.GetTag() == "PResart")
				{
					Graphic::mcamera->SetZoomLevel(0.25f);
					p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");// restart the game in pause
				}

				if (ID_tag.GetTag() == "Mute")
				{
					p_Audio->SetVolumeByChannel(p_Audio->GetMasterChannelGroup(), 0.0f); //tell the system to mute
				}

				if (ID_tag.GetTag() == "Back")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Menu.json");// return back to the main menu
				}

				if (ID_tag.GetTag() == "No")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Menu.json");// return back  to the main menu from quit

				}

				if (ID_tag.GetTag() == "Yes")
				{
					p_GUI->toggle_script();// tell the system to end the game 
				}

				int randSoundIndex{ 0 };
				if (p_ecs.HaveComponent<Audio>(GetScriptEntityID())&& p_ecs.GetComponent<Audio>(GetScriptEntityID()).IsRandomized())
				{
					randSoundIndex = Random::Range(0, p_ecs.GetComponent<Audio>(GetScriptEntityID()).GetSize() - 1);
				}

				if (ID_tag.GetTag() == "FlamingoBtn")
				{
					
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[randSoundIndex].should_play = true;
					p_Scene->setSceneToLoad("Assets/Scene/FlamingoScene.json");// switch to flamingo scene
				}

				if (ID_tag.GetTag() == "SealBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[randSoundIndex].should_play = true;
					p_Scene->setSceneToLoad("Assets/Scene/SealScene.json");// switch to seal scene
				}

				if (ID_tag.GetTag() == "HorseBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[randSoundIndex].should_play = true;
					p_Scene->setSceneToLoad("Assets/Scene/HorseScene.json");// switch to horse scene
				}

				if (ID_tag.GetTag() == "OwlBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[randSoundIndex].should_play = true;
					p_Scene->setSceneToLoad("Assets/Scene/OwlScene.json");// switch to owl scene
				}

				if (ID_tag.GetTag() == "AltFlamingoBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
					p_GameManager.SetSceneToSwitch("Flamingo");// switch to flamingo scene
				}

				if (ID_tag.GetTag() == "AltSealBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
					p_GameManager.SetSceneToSwitch("Seal");
				}

				if (ID_tag.GetTag() == "AltHorseBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
					p_GameManager.SetSceneToSwitch("Horse");// switch to horse scene
				}

				if (ID_tag.GetTag() == "AltOwlBtn")
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true;
					p_GameManager.SetSceneToSwitch("Owl");// switch to owl scene
				}

				if (ID_tag.GetTag() == "OverviewTab")
				{
					p_Scene->setSceneToLoad("Assets/Scene/HowToPlay.json");
				}

				if (ID_tag.GetTag() == "FlamingoTab")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Manual_Flamingo.json");
				}
				if(ID_tag.GetTag()=="SealTab")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Manual_Seal.json");
				}
				if (ID_tag.GetTag() == "HorseTab")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Manual_Horse.json");
				}
				if (ID_tag.GetTag() == "OwlTab")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Manual_Owl.json");
				}
				if (ID_tag.GetTag() == "XButton")
				{
					p_Scene->setSceneToLoad("Assets/Scene/Menu.json");
				}
			}

			if (curr_state == button_state::click)
			{
				if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && ((p_ecs.GetComponent<Audio>(GetScriptEntityID())).GetSize() > 1) && (p_GUI->Check_HTP() == false))
				{
					p_ecs.GetComponent<Audio>(GetScriptEntityID())[0].should_play = true; //menu click audio
				}
				//when mouse is held down set the coresspoding sprites to clicked state
				if (ID_tag.GetTag() == "Start" && p_GUI->Check_HTP() == false)
				{
					//	p_GUI->toggle_menu();
					//spt.SetTexture("UI/Start_Click");
					//transform.ScaleBy(-0.1f);
				}

				if (ID_tag.GetTag() == "Quit" && p_GUI->Check_HTP() == false)
				{
					//spt.SetTexture("UI/Quit_Click");
					//transform.ScaleBy(-0.1f);
				}

				if (ID_tag.GetTag() == "MainMenu" && p_GUI->Check_HTP() == false)
				{
					spt.SetTexture("UI/MM_R");

				}

				if (ID_tag.GetTag() == "Restart" && p_GUI->Check_HTP() == false)
				{
					spt.SetTexture("UI/RESTART_R");

				}

				if (ID_tag.GetTag() == "HowToPlay")
				{
					//spt.SetTexture("UI/htp_button_Click");
				}

				if (ID_tag.GetTag() == "Options" && p_GUI->Check_HTP() == false)
				{
					//spt.SetTexture("UI/Options_Click");
					//transform.ScaleBy(-0.1f);
				}

				if (ID_tag.GetTag() == "Resume")
				{
					spt.SetTexture("UI/Resume_Click");

				}
				if (ID_tag.GetTag() == "PRestart")
				{
					spt.SetTexture("UI/Restart_Click");
				}

				if (ID_tag.GetTag() == "Mute")
				{
					spt.SetTexture("UI/Mute_Click");
				}

				if (ID_tag.GetTag() == "Back")
				{
					spt.SetTexture("UI/BACK_R");
				}


				if (ID_tag.GetTag() == "No")
				{
					spt.SetTexture("UI/BlankArrow");
				}

				if (ID_tag.GetTag() == "Yes")
				{
					spt.SetTexture("UI/BlankArrow");
				}

			}


		}
		// when the button is not interacted with set buttons to their respective idle state
		else
		{
			initial_hover = true;

			if (ID_tag.GetTag() == "Start")
			{
				spt.SetTexture("UI/BlankArrow");
				if (leftcollider)
				{
					transform.ScaleBy(1.f/1.2f);
					leftcollider = false;
				}
				//
			}

			if (ID_tag.GetTag() == "Quit")
			{
				spt.SetTexture("UI/BlankArrow");
				if (leftcollider)
				{
					transform.ScaleBy(1.f / 1.2f);
					leftcollider = false;
				}
			}
			if (ID_tag.GetTag() == "Options")
			{
				spt.SetTexture("UI/BlankArrow");
				if (leftcollider)
				{
					transform.ScaleBy(1.f / 1.2f);
					leftcollider = false;
				}
			}

			if (ID_tag.GetTag() == "MainMenu")
			{
				spt.SetTexture("UI/MM_Y");
			}

			if (ID_tag.GetTag() == "Restart")
			{
				spt.SetTexture("UI/RESTART_Y");
			}
			if (ID_tag.GetTag() == "RestartT")
			{
				spt.SetTexture("Characters/Blank");
			}
			if (ID_tag.GetTag() == "QuitT")
			{
				spt.SetTexture("Characters/Blank");
			}
			if (ID_tag.GetTag() == "ProceedT")
			{
				spt.SetTexture("Characters/Blank");
			}

			if (ID_tag.GetTag() == "HowToPlay")
			{
				spt.SetTexture("UI/BlankArrow");
				if (leftcollider)
				{
					transform.ScaleBy(1.f / 1.2f);
					leftcollider = false;
				}
			}

			if (ID_tag.GetTag() == "Resume")
			{
				spt.SetTexture("UI/Resume_Idle");
			}
			if (ID_tag.GetTag() == "PRestart")
			{
				spt.SetTexture("UI/Restart_Idle");
			}
			if (ID_tag.GetTag() == "Back")
			{
				spt.SetTexture("UI/BACK_Y");
			}

			if (ID_tag.GetTag() == "No")
			{
				spt.SetTexture("UI/BlankArrow");

				if (leftcollider)
				{
					transform.ScaleBy(1.f / 1.2f);
					leftcollider = false;
				}
			}

			if (ID_tag.GetTag() == "Yes")
			{
				spt.SetTexture("UI/BlankArrow");

				if (leftcollider)
				{
					transform.ScaleBy(1.f / 1.2f);
					leftcollider = false;
				}
			}
		}

	}

	void ButtonResponse::End()
	{

	}

	ButtonResponse* ButtonResponse::Clone() const
	{
		return new ButtonResponse(*this);
	}


	std::string ButtonResponse::GetScriptName()
	{
		return "ButtonResponse";
	}

	bool ButtonResponse::is_within_box(glm::vec2 cur, Collider box, Transform trans)//unique aabb code for entity and mouse
	{
		vec2D mPos{ cur.x,cur.y };
		for (int i = 0; i < 2 ; i++)
		{
			vec2D offsetPos{ trans.GetPos().x + box[i].mOffset.x ,trans.GetPos().y + box[i].mOffset.y };
			switch (box[i].mCol)
			{
			case Collider::ColliderType::circle:
			case Collider::ColliderType::bubble:
				if (distance(mPos, offsetPos) < box[i].mRadius) {
					return true;
				}
				break;
			case Collider::ColliderType::rect:
			case Collider::ColliderType::box:
				vec2D minPt{ offsetPos.x - box[i].mMin.x,offsetPos.y - box[i].mMin.y }, maxPt{ offsetPos.x + box[i].mMax.x,offsetPos.y + box[i].mMax.y };
				if (mPos.x > minPt.x && mPos.x<maxPt.x && mPos.y>minPt.y && mPos.y < maxPt.y)
				{
					return true;
					//std::cout << "entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " collider check\n";

				}
				break;
			}
		}
		return false;
	}
}