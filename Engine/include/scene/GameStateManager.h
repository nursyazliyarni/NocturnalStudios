/*****************************************************************//**
 * \file   GameStateManager.h
 * \brief  game
 * 
 * \author rahra
 * \date   October 2025
 *********************************************************************/

#include "../include/Scene/SceneManager.h"

namespace{

}
class GameStateManager{
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
	int innerState{0};
	LEVEL currLvl{ NONE }, nextLevel{ NONE };

	void MainMenuState();
	void LoadState();
	void PlayState();


public:
	void StateMachine();

};
