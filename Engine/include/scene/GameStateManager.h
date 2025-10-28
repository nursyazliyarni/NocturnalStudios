/*****************************************************************//**
 * \file   GameStateManager.h
 * \brief  game
 *
 * \author rahra
 * \date   October 2025
 *********************************************************************/

#include "../include/Scene/SceneManager.h"
#include "graphics/Window.h"

class GameStateManager {
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
	SceneManager& sceneManager{ SceneManager::GetSceneManager() };
	Window& window;

	void MainMenuState();
	void LoadState();
	void PlayState();
	void QuitGame();

	GameStateManager(Window&);

	GameStateManager(const GameStateManager&) = delete;
	GameStateManager& operator=(const GameStateManager&) = delete;

	GameStateManager(GameStateManager&&) = delete;
	GameStateManager& operator=(GameStateManager&&) = delete;
public:
	void StateMachine();

	static GameStateManager& getGSM(Window& w) {
		static GameStateManager instance(w);
		return instance;
	}

};
