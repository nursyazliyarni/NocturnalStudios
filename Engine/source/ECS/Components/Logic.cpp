/*!------------------------------------------------------------------------------------------------
    File: Logic.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize logic components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the logic class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Logic.h"
#include "Scripts/ButtonResponse.h"
#include "Scripts/ScenerioScript.h"
#include "Scripts/SliderScript.h"
#include "Scripts/AudioManager.h"
#include "Scripts/PauseMenu.h"
#include "Scripts/BackgroundAudio.h"
#include "Scripts/CutScene.h"
#include "Scripts/DialogueManager.h"
#include "Scripts/DragAndDrop.h"
#include "Scripts/HealthBar.h"
#include "Scripts/AnimalLogic.h"
#include "Scripts/TimerDisplay.h"


namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for Logic Component
	****************************************************************************/
	Logic::Logic() { entityID = GetComponentEntityID(); }

	/*!*************************************************************************
	Copy Ctor
	****************************************************************************/
	Logic::Logic(const Logic& rhs)
	{
		mScriptNameVector = rhs.mScriptNameVector;
		mScriptsVector.resize(rhs.mScriptsVector.size());
		for (size_t i = 0; i < mScriptsVector.size(); ++i)
		{
			mScriptsVector[i] = rhs.mScriptsVector[i]->Clone();
			mScriptsVector[i]->SetScriptEntityID(this->GetComponentEntityID());
		}
	}
	/*!*************************************************************************
	Copy Assignment for operator overload
	****************************************************************************/
	Logic& Logic::operator= (const Logic& rhs)
	{
		if (this == &rhs) return *this;
		// delete existing scripts first
		for (auto* s : mScriptsVector) { delete s; }
		mScriptsVector.clear();

		mScriptNameVector = rhs.mScriptNameVector;
		mScriptsVector.resize(rhs.mScriptsVector.size());
		for (size_t i = 0; i < mScriptsVector.size(); ++i)
		{
			mScriptsVector[i] = rhs.mScriptsVector[i]->Clone();
			mScriptsVector[i]->SetScriptEntityID(this->GetComponentEntityID());
		}
		return *this;
	}

	// Move ctor
	Logic::Logic(Logic&& rhs) noexcept
	{
		mScriptNameVector = std::move(rhs.mScriptNameVector);
		mScriptsVector = std::move(rhs.mScriptsVector);
		rhs.mScriptsVector.clear();
	}
	// Move assignment
	Logic& Logic::operator=(Logic&& rhs) noexcept
	{
		if (this == &rhs) return *this;
		for (auto* s : mScriptsVector) { delete s; }
		mScriptsVector.clear();
		mScriptNameVector = std::move(rhs.mScriptNameVector);
		mScriptsVector = std::move(rhs.mScriptsVector);
		rhs.mScriptsVector.clear();
		return *this;
	}

	/*!*************************************************************************
	Sets the Script's entity to retrieve data only from that entity
	****************************************************************************/
	void Logic::SetScriptEntity(Entity entity)
	{
		this->SetComponentEntityID(entity);
		for (auto i = mScriptsVector.begin(); i != mScriptsVector.end(); ++i)
		{
			(*i)->SetScriptEntityID(this->GetComponentEntityID());
		}
	}

	/*!*************************************************************************
	Deserialize for Logic Component
	****************************************************************************/
	bool Logic::Deserialize(const rapidjson::Value& obj)
	{
		int scriptCount = obj.HasMember("ScriptCount") && obj["ScriptCount"].IsNumber() ? obj["ScriptCount"].GetInt() : 0;
		if (scriptCount <= 0)return true;
		for (int i = 0; i < obj["ScriptCount"].GetInt(); ++i)
		{
			mScriptNameVector.push_back(obj[std::to_string(i).c_str()].GetString());
		}
		for (size_t j = 0; j < mScriptNameVector.size(); ++j)
		{
			if (mScriptNameVector[j] == "ButtonResponse")
			{
				mScriptsVector.push_back(new ButtonResponse());
			}
			if (mScriptNameVector[j] == "ScenerioScript")
			{
				mScriptsVector.push_back(new ScenerioScript());
			}
			if (mScriptNameVector[j] == "AudioManager")
			{
				mScriptsVector.push_back(new AudioManager());
			}
			if (mScriptNameVector[j] == "BackgroundAudio")
			{
				mScriptsVector.push_back(new BackgroundAudio());
			}
			if (mScriptNameVector[j] == "SliderScript")
			{
				mScriptsVector.push_back(new SliderScript());
			}
			if (mScriptNameVector[j] == "PauseMenu")
			{
				mScriptsVector.push_back(new PauseMenu());
			}
			if (mScriptNameVector[j] == "CutScene")
			{
				mScriptsVector.push_back(new CutScene());
			}
			if (mScriptNameVector[j] == "DialogueManager")
			{
				mScriptsVector.push_back(new DialogueManager());
			}
			if (mScriptNameVector[j] == "DragAndDrop")
			{
				mScriptsVector.push_back(new DragAndDrop());
			}
			if (mScriptNameVector[j] == "HealthBar")
			{
				mScriptsVector.push_back(new HealthBar());
			}
			if (mScriptNameVector[j] == "AnimalLogic")
			{
				mScriptsVector.push_back(new AnimalLogic());
			}
			if (mScriptNameVector[j] == "TimerDisplay")
			{
				mScriptsVector.push_back(new TimerDisplay());
			}
		}
		return true;
	}

	/*!*************************************************************************
	Serialize for Logic Component
	****************************************************************************/
	bool Logic::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		int i = 0;
		for (i = 0; i < mScriptNameVector.size(); ++i)
		{
			writer->Key(std::to_string(i).c_str());
			writer->String(mScriptNameVector[i].c_str());
		}
		writer->Key("ScriptCount");
		writer->Int(i);
		return true;
	}

	/*!*************************************************************************
	Inserts the scripts into the vector to loop through
	****************************************************************************/
	void Logic::InsertScript(IScript* script, Entity entity)
	{
		this->SetComponentEntityID(entity);
		script->SetScriptEntityID(this->GetComponentEntityID());
		mScriptNameVector.push_back(script->GetScriptName());
		mScriptsVector.push_back(script);
	}

	void Logic::DeleteScript(std::string scriptname)
	{
		for (int idx = 0;idx < mScriptNameVector.size(); idx++) {
			if (mScriptNameVector[idx] == scriptname) {
				// free the script instance too
				delete mScriptsVector[idx];
				mScriptNameVector.erase(mScriptNameVector.begin() + idx);
				mScriptsVector.erase(mScriptsVector.begin() + idx);
				break;
			}
		}
	}

	/*!*************************************************************************
	Clear all Scripts inside the Vector
	****************************************************************************/
	void Logic::ClearAllScripts()
	{
		for (auto* s : mScriptsVector)
		{
			if (s) { s->End(); delete s; }
		}
		mScriptsVector.clear();
		mScriptNameVector.clear();
	}

	IScript* Logic::GetScriptByName(std::string ScriptName)
	{
		for (size_t i = 0; i < mScriptNameVector.size(); ++i)
		{
			if (mScriptNameVector[i] == ScriptName)
			{
				return mScriptsVector[i];
			}
		}
		return nullptr;
	}

	/*!*************************************************************************
	Destructor for Logic Component
	****************************************************************************/
	Logic::~Logic()
	{
		for (auto* script : mScriptsVector)
		{
			if (script != nullptr)
			{
				delete script;
			}
		}

		mScriptsVector.clear();
		mScriptNameVector.clear();
	}

	/*!*************************************************************************
	End all scripts inside the vector
	****************************************************************************/
	void Logic::EndAllScripts()
	{
		for (auto* script : mScriptsVector)
		{
			if (script != nullptr)
			{
				script->End();
			}
		}
	}
}