/*!------------------------------------------------------------------------------------------------
    File: WinData.h
    Project: NocturnalStudio

	Brief:
		Window Component used to stored data regarding the window such as
		height and width of the window
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the WinData.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	//Window Component
	class WinData : public JSONSerializer
	{
	public:
		WinData();
		~WinData() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		std::string GetTitle() { return mTitle; }
		unsigned int GetWidth() { return mWidth; }
		unsigned int GetHeight() { return mHeight; }
		unsigned int GetCurrWidth() { return mCurrentWidth; }
		unsigned int GetCurrHeight() { return mCurrentHeight; }

		void SetTitle(std::string Title) { mTitle = Title; }
		void SetWidth(unsigned int Width) { mWidth = Width; }
		void SetHeight(unsigned int Height) { mHeight = Height; }
		void SetCurrWidth(unsigned int CurrWidth) { mCurrentWidth = CurrWidth; }
		void SetCurrHeight(unsigned int CurrHeight) { mCurrentHeight = CurrHeight; }

	private:
		std::string mTitle;
		unsigned int mWidth,
			mHeight,
			mCurrentWidth,
			mCurrentHeight;
		float MasterVol;
		float BGMVol;
		float SFXVol;
	};
}