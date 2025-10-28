/*****************************************************************//**
 * \file   GameStateManager.h
 * \brief  game
 *
 * \author rahra
 * \date   October 2025
 *********************************************************************/

#include "../include/Scene/GameStateManager.h"

void GameStateManager::StateMachine() {
	switch (currState)
	{
	case MAIN_MENU:
		MainMenuState();
		break;
	case LOAD:
		LoadState();
		break;
	case PAUSE:
		break;
	case PLAY:
		PlayState();
		break;
	case WIN:
		break;
	case LOSE:
		break;
	case QUIT:
		break;
	default:
		break;
	}
}

void GameStateManager::MainMenuState() {
	if (innerState == 0)
	{
		//do whatever u need to do when first loading in main menu. 
		//call up the scene manager to change scene to main menu?
		innerState++; //move to the next state after execution
	}
	if (innerState == 1)
	{
		//render main menu, probably call up scenemanager.render or smth
		if (false)//player press esc to exit
		{
			nextState = QUIT;
			innerState++; //this increments the innerState
		}

	}
	if (innerState == 2)
	{
		currState = nextState;
		innerState = 0;//reset innerstate to entry state
	}

}
void GameStateManager::LoadState() {
	if (innerState == 0)
	{
		//ngl idk what u plan to do for loading, but i made a state for it 
		innerState++; //move to the next state after execution
	}
	if (innerState == 1)
	{
		//probably not needed for loading state
		if (true)//all files loaded or smth
		{
			nextState = MAIN_MENU; //or the saved level
			innerState++; //this increments the innerState
		}

	}
	if (innerState == 2)
	{
		//clean up anything here if needed
		currState = nextState;
		innerState = 0;//reset innerstate to entry state
	}
}
void GameStateManager::PlayState() {
	if (innerState == 0)
	{
		//setup the scene
		innerState++; //move to the next state after execution
	}
	if (innerState == 1)
	{

		if (true)//win condition for the level fulfiled
		{
			switch (currLvl)
			{
			case GameStateManager::TUTORIAL:
				nextLevel = STAGE_1;
				break;
			case GameStateManager::STAGE_1:
				nextLevel = STAGE_2;
				break;
			case GameStateManager::STAGE_2:
				nextLevel = STAGE_3;
				break;
			case GameStateManager::STAGE_3:
				nextLevel = NONE;
				break;
			default:
				break;
			}
			innerState++; //this increments the innerState
		}
		if (false) //lose condition
		{
			nextState = LOSE;
			innerState++;
		}

	}
	if (innerState == 2)
	{
		if (currState == nextState)
		{
			currLvl = nextLevel;
			if (currLvl == NONE)
			{
				nextState == WIN;
			}
		}

		innerState = 0;//reset innerstate to entry state
	}
}
