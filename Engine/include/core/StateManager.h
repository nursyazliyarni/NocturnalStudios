/*****************************************************************//**
 * \file   StateManager.h
 * \brief  game
 *
 * \author rahra
 * \date   October 2025
 *********************************************************************/

#include "../include/Scene/SceneManager.h"
#include "graphics/Window.h"

class StateManager {
private:
	enum PLAYERSTATE
	{
		MAIN_MENU,
		LOAD,
		PAUSE,
		PLAY,
		WIN,
		LOSE,
		QUIT
	};
	enum LEVEL
	{
		NONE,
		TUTORIAL,
		STAGE_1,
		STAGE_2,
		STAGE_3
	};
	PLAYERSTATE currState{ MAIN_MENU }, nextState{ MAIN_MENU };
	int innerState{ 0 };
	LEVEL currLvl{ NONE }, nextLevel{ NONE };
	SceneManager& sceneManager= SceneManager::GetSceneManager() ;
	Window& window;

	void MainMenuState();
	void LoadState();
	void PlayState();
	void QuitGame();

	StateManager(Window&);

	StateManager(const StateManager&) = delete;
	StateManager& operator=(const StateManager&) = delete;

	StateManager(StateManager&&) = delete;
	StateManager& operator=(StateManager&&) = delete;
public:
	void StateMachine();

	static StateManager& getGSM(Window& w) {
		static StateManager instance(w);
		return instance;
	}

};
