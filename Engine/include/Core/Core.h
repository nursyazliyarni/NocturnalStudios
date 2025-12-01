
#pragma once
#include <memory>

//Engine into dll
#ifdef NOCTURNAL_PLATFORM_WINDOWS
	#ifdef NOCTURNAL_BUILD_DLL
		#define NOCTURNAL_ENGINE_API __declspec(dllexport)
	#else
		#define NOCTURNAL_ENGINE_API __declspec(dllimport)
	#endif 
#else
	#error support Windows only!
#endif

#define BIT(x) (x << 1)
#define GETTEXTURE(x) ResourceManager::GetTexture(x)

namespace NocturnalEngine {

	template<typename T>
	using Singleton = std::unique_ptr<T>;
	template<typename T, typename ... Paras>
	constexpr Singleton<T> CreateSingleton(Paras&& ... paras)
	{
		return std::make_unique<T>(std::forward<Paras>(paras)...);
	}

	template<typename T>
	using MultiRefs = std::shared_ptr<T>;
	template<typename T, typename ... Paras>
	constexpr MultiRefs<T> CreateMultiRefs(Paras&& ... paras)
	{
		return std::make_shared<T>(std::forward<Paras>(paras)...);
	}
}