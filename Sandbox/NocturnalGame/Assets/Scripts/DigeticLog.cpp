/*!------------------------------------------------------------------------------------------------
	File: DigeticLog.cpp
	Project: NocturnalStudio

	Brief:
		The DigeticLog system manages the displaying of the ingame log which tracks animals statuses
		low hwalth, death etc

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the DigeticLog class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/DigeticLog.h"
#include "ECS/ECS.h"
#include "Scripts/GlobalGameManager.h"


namespace NocturnalEngine {

	void DigeticLog::Start()
	{
		if (p_ecs.HaveComponent<Transform>(entityID)&&p_ecs.HaveComponent<Text>(entityID))
		{
			textWorldHeight = p_ecs.GetComponent<Text>(entityID).GetSize() * 0.0020f;
			float yBuffer = p_ecs.GetComponent<Transform>(entityID).GetMaxPos().y-(p_ecs.GetComponent<Transform>(entityID).GetPos().y+p_ecs.GetComponent<Text>(entityID).GetPosition()[1])-textWorldHeight;
			logSpaces = static_cast<int>((p_ecs.GetComponent<Transform>(entityID).GetScale().y-yBuffer*2.f) /textWorldHeight);


			logTxtObjs.clear();
			logTxtObjs.push_back(entityID);
			for (int i{ 1 }; i < logSpaces; i++)
			{
				logTxtObjs.push_back(p_ecs.CreateEntity());
				p_ecs.AddComponent<Transform>(logTxtObjs[i], p_ecs.GetComponent<Transform>(entityID));
				p_ecs.GetComponent<Transform>(logTxtObjs[i]).GetPos().y -= textWorldHeight * static_cast<float>(i);
				p_ecs.AddComponent<Text>(logTxtObjs[i], p_ecs.GetComponent<Text>(entityID));
				p_ecs.GetComponent<Text>(logTxtObjs[i]).SetContent("");
			}
			p_ecs.GetComponent<Text>(entityID).SetContent("");

			const std::deque<std::string>& queueRef{ p_GameManager.GetLogEntries() };

			int maxlogs = static_cast<int>(queueRef.size()) < logSpaces ? static_cast<int>(queueRef.size()) : logSpaces,
				startingLog = static_cast<int>(queueRef.size()) < logSpaces ? 0 : static_cast<int>(queueRef.size()) - logSpaces;

			for (int i{ 0 }; i < maxlogs; i++, startingLog++)
			{

				p_ecs.GetComponent<Text>(logTxtObjs[i]).SetContent(queueRef[startingLog]);
			}
			setupDone = true;
		}
		
	}

	void DigeticLog::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);

		if (!setupDone)Start();

		if (!p_GameManager.HasLogBeenUpdated())
		{
			const std::deque<std::string>& queueRef{ p_GameManager.GetLogEntries() };

			int maxlogs = static_cast<int>(queueRef.size()) < logSpaces ? static_cast<int>(queueRef.size()) : logSpaces,
				startingLog = static_cast<int>(queueRef.size()) < logSpaces ? 0 : static_cast<int>(queueRef.size()) - logSpaces;

			for (int i{ 0 }; i < maxlogs; i++,startingLog++)
			{
				
				p_ecs.GetComponent<Text>(logTxtObjs[i]).SetContent(queueRef[startingLog]);
			}
			
		}

	}

	void DigeticLog::End()
	{
		for (Entity txtObjs:logTxtObjs)
		{
			if (txtObjs!=entityID)p_ecs.DestroyEntity(txtObjs);
		}
		logTxtObjs.clear();
		setupDone = false;
	}




}
