/*!------------------------------------------------------------------------------------------------
	File: ButtonComponent.h
	Project: NocturnalStudio

	Brief:
		Component for determining how a button entity  responds

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the ButtonComponent.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS/Components/IComponent.h"
#include "pch.h"


namespace NocturnalEngine
{
	class ButtonComponent : public IComponent
	{
	public:
		enum class ButtonFlag {
			NONE = 0,
			GOTO_SCENE = 1 << 0,  // Load a target scene
			AUDIO_ON_HOVER = 1 << 1,  // Play sound on hover
			AUDIO_ON_CLICK = 1 << 2,  // Play sound on click 
			TEX_CHANGE_ON_HOVER = 1 << 3,  // Swap texture on hover
			TEX_CHANGE_ON_CLICK = 1 << 4,  // Swap texture on click
			SCALE_ON_HOVER = 1 << 5,  // Scale up on hover, revert on leave
			SET_GAME_STATE = 1 << 6,  // Set a GameState (PLAYING, etc.)
			RESET_STATS = 1 << 7,  // Call ResetStats()
			NEXT_LEVEL = 1 << 8,  // ...
			SET_CAMERA_ZOOM = 1 << 9, // Set camera zoom level
			MUTE_AUDIO = 1 << 10, // Mute all audio channels
			ALT_SCENE_SWITCH = 1 << 11, // SetSceneToSwitch() (Alt buttons)
			EXIT_GAME = 1 << 12  // Call toggle_script to quit
		};
		std::string gotoScene{};
		bool hoverAudioRandom{ false }, clickAudioRandom{ false };
		int hoverAudioIndex{ 0 }, clickAudioIndex{ 0 };
		std::string hoverTexture{}, clickTexture{};
		float hoverScale{};
		enum class GameManState {
			STARTING=0,
			PLAYING=1,
			ENDING=2,
			PAUSED=3
		};
		GameManState stateToSet{};
		float cameraZoom{};
		std::string altSceneSwitch{};

		int Flag() const { return flag; }

		void ClearFlag() { flag = 0; }

		ButtonComponent();
		~ButtonComponent() = default;

		virtual bool Deserialize(const rapidjson::Value& obj);

		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		virtual bool ToEditor() override;

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

		

	private:
		int flag{ 0 };

		
	};
	inline bool operator&(int flags, ButtonComponent::ButtonFlag check) {
		return (flags & static_cast<int>(check)) != 0;
	}
	inline int operator|(int flags, ButtonComponent::ButtonFlag check) {
		return flags | static_cast<int>(check);
	}
	inline int operator|=(int& flags, ButtonComponent::ButtonFlag check) {
		return flags |= static_cast<int>(check);
	}
	inline int operator&=(int& flags, ButtonComponent::ButtonFlag check) {
		return flags &= static_cast<int>(check);
	}
	
}
