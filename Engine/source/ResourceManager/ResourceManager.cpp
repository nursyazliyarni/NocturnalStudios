/*!------------------------------------------------------------------------------------------------
    File: ResourceManager.cpp
    Project: NocturnalStudio

	Brief:
		This file contain the defintion for ResourceManager class.
		Resource manager can load textures, shader, fonts, icons
		and audios.
	
    Primary Author:
         LAU XIANG YANG (95%)
		- Created the ResourceManager class and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Added font/text resource support for text rendering.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ResourceManager/ResourceManager.h"

namespace NocturnalEngine {
	std::filesystem::path EngineAssetPath{std::filesystem::current_path().parent_path().parent_path()/"Resources"/"EngineAssets"};
	std::unordered_map <std::string, std::shared_ptr<Texture>> ResourceManager::textures;
	std::unordered_map <std::string, std::shared_ptr<Shader>> ResourceManager::shaders;
	std::unordered_map <std::string, std::shared_ptr<Font>> ResourceManager::fonts;
	std::unordered_map <std::string, std::shared_ptr<CAudioEngine>> ResourceManager::audios;
	std::unordered_map <std::string, std::shared_ptr<Texture>> ResourceManager::Icons;

	/*!*************************************************************************
	Load texture
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::LoadTexture(std::string name, const std::string& filepath, bool loadByNearest)
	{
		return textures[name] = ResourceManager::LoadTextureFromFile(filepath,loadByNearest);
	}

	/*!*************************************************************************
	Retrieve texture based on filename
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::GetTexture(std::string name)
	{
		return textures[name];
	}

	/*!*************************************************************************
	Load texture from filepath
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::LoadTextureFromFile(const std::string& filepath, bool loadByNearest )
	{
		auto m_texture = std::make_shared<Texture>();
		m_texture->GenTexture(filepath,loadByNearest);
		return m_texture;
	}

	/*!*************************************************************************
	Generate texture
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::GenTexture(unsigned int width, unsigned int height)
	{
		auto m_texture = std::make_shared<Texture>();
		m_texture->GenTexture(width, height);
		return m_texture;
	}

	/*!*************************************************************************
	Load Icons from file
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::LoadIcons(std::string name, const std::string& filepath)
	{

		return Icons[name] = ResourceManager::LoadIconsFromFile(filepath);
	}

	/*!*************************************************************************
	Retrieve icons based on filename
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::GetIcon(std::string name)
	{
		return Icons[name];
	}

	/*!*************************************************************************
	Load shader based on filepath
	****************************************************************************/
	std::shared_ptr<Shader> ResourceManager::LoadShaderFromFile(const std::string& filepath)
	{
		auto m_shader = std::make_shared<Shader>(filepath);
		
		return m_shader;
	}


	std::shared_ptr<Shader> ResourceManager::LoadShaderFromFile(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
	{
		std::ifstream vFile(vertexPath);
		std::ifstream fFile(fragmentPath);

		if (!vFile.is_open())
		{
			NOCTURNAL_ENGINE_ERROR("Failed to open vertex shader file: {}", vertexPath);
			return nullptr;
		}

		if (!fFile.is_open())
		{
			NOCTURNAL_ENGINE_ERROR("Failed to open fragment shader file: {}", fragmentPath);
			return nullptr;
		}

		std::stringstream vStream, fStream;
		vStream << vFile.rdbuf();
		fStream << fFile.rdbuf();

		vFile.close();
		fFile.close();

		return std::make_shared<Shader>(name, vStream.str(), fStream.str());
	}

	std::shared_ptr<Shader> ResourceManager::LoadShader(std::string name, const std::string& vertexPath, const std::string& fragmentPath)
	{
		return shaders[name] = ResourceManager::LoadShaderFromFile(name, vertexPath, fragmentPath);
	}


	/*!*************************************************************************
	Retrieve texture based on filename and filepath
	****************************************************************************/
	std::shared_ptr<Shader> ResourceManager::LoadShader(std::string name, const std::string& filepath)
	{
		return shaders[name] = ResourceManager::LoadShaderFromFile(filepath);
	}

	/*!*************************************************************************
	Retrieve shader on filename 
	****************************************************************************/
	std::shared_ptr<Shader> ResourceManager::GetShader(std::string name)
	{
		return shaders[name];
	}

	/*!*************************************************************************
	Load font based on filepath
	****************************************************************************/
	std::shared_ptr<Font> ResourceManager::LoadFont(std::string name, const std::string& filepath, unsigned int fontsize)
	{
		return fonts[name] = ResourceManager::LoadFontFromFile(filepath, fontsize);
	}

	/*!*************************************************************************
	Retrieve font based on name
	****************************************************************************/
	std::shared_ptr<Font> ResourceManager::GetFont(const std::string& name)
	{
		auto it = fonts.find(name);
		if (it == fonts.end())
			return nullptr;
		return it->second;
	}

	/*!*************************************************************************
	Load font based on filepath
	****************************************************************************/
	std::shared_ptr<Font> ResourceManager::LoadFontFromFile(const std::string& filepath, unsigned int fontsize)
	{
		auto m_font = std::make_shared<Font>();
		m_font->Init();
		m_font->Load(filepath, fontsize);
		return m_font;
	}

	/*!*************************************************************************
	Load icons based on filepath
	****************************************************************************/
	std::shared_ptr<Texture> ResourceManager::LoadIconsFromFile(const std::string& filepath)
	{
		auto mIcons = std::make_shared<Texture>();
		mIcons->GenTexture(filepath);
		return mIcons;
	}

	/*!*************************************************************************
	Load audio based on filepath
	****************************************************************************/
	std::shared_ptr<CAudioEngine> ResourceManager::LoadAudioFromFile(const std::string& filepath)
	{
		auto m_audio = std::make_shared<CAudioEngine>();
		m_audio->Loadsound(filepath);
		return m_audio;
	}

	/*!*************************************************************************
	Retrieve audio based on filename
	****************************************************************************/
	std::shared_ptr<CAudioEngine> ResourceManager::GetAudio(std::string name)
	{
		return audios[name];
	}

	/*!*************************************************************************
	Load audio
	****************************************************************************/
	std::shared_ptr<CAudioEngine> ResourceManager::LoadAudio(std::string name, const std::string& filepath)
	{
		UNREFERENCED_PARAMETER(name);
		(void)filepath; 
		return std::shared_ptr<CAudioEngine>();
	}

	/*!*************************************************************************
	Clear various maps for resources being managed
	****************************************************************************/
	void NocturnalEngine::ResourceManager::clear()
	{
		textures.clear();
		Icons.clear();
		shaders.clear();
		fonts.clear();
	}
	void ResourceManager::SetEngineResources() {

		std::filesystem::path exePath = std::filesystem::current_path();
		if (exePath.has_extension()&&exePath.extension().string() ==".exe")
		{
			 exePath = std::filesystem::current_path().parent_path();
		}
		std::filesystem::path relativeToExe = exePath / "Assets"/"Resources"/"EngineAssets";

		if (std::filesystem::exists(relativeToExe)) {
			EngineAssetPath = relativeToExe;
			return;
		}

		// Second try: traverse up to find NocturnalStudios (running from VS)
		std::filesystem::path search = std::filesystem::current_path();
		while (search.filename() != "NocturnalStudios") {
			if (search == search.parent_path()) return; // hit root, not found
			search = search.parent_path();
		}
		EngineAssetPath = search / "Resources" / "EngineAssets";
	}

}
