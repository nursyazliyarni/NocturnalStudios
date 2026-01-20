/*!------------------------------------------------------------------------------------------------
    File: Log.h
    Project: NocturnalStudio

	Brief:
		This file contains the declaration of Log class. This file holds the assert system for
		the project
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the log class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

#ifdef _MSC_VER
// Supress warning: not sure whether we are going to dllexport out this log
#pragma warning( disable : 4251 )
#endif
namespace NocturnalEngine {
	template class NOCTURNAL_ENGINE_API std::shared_ptr<spdlog::logger>;
	class NOCTURNAL_ENGINE_API Log
	{
	public:
		
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		static std::string GetImguiLog();

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::ostringstream oss;
	};
}
//Macros for logger
//Core aka Engine
//Trace is to track variable changes during execution like Window Size etc
#define NOCTURNAL_ENGINE_TRACE(...)	 ::NocturnalEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define NOCTURNAL_ENGINE_INFO(...)	 ::NocturnalEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define NOCTURNAL_ENGINE_WARN(...)	 ::NocturnalEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define NOCTURNAL_ENGINE_ERROR(...)	 ::NocturnalEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define NOCTURNAL_ENGINE_CRITICAL(...) ::NocturnalEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

//Assertion failed, it will track where it happened and print it in console (for debug)
#define NOCTURNAL_ENGINE_ASSERT(x, ...) {if((x)) { NOCTURNAL_ENGINE_CRITICAL("Assert Failed: {} {}\n\tIn File:{}\n\tLine: {}",x, __VA_ARGS__,__FILE__,__LINE__);__debugbreak();}}

//Client aka Game Application
#define NOCTURNAL_GAME_TRACE(...) ::NocturnalEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define NOCTURNAL_GAME_INFO(...)  ::NocturnalEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define NOCTURNAL_GAME_WARN(...)  ::NocturnalEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define NOCTURNAL_GAME_ERROR(...) ::NocturnalEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define NOCTURNAL_GAME_CRITICAL(...) ::NocturnalEngine::Log::GetClientLogger()->critical(__VA_ARGS__)
