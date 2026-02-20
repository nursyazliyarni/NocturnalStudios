/*!------------------------------------------------------------------------------------------------
	File: NewProjectMaker.h
	Game Project: NocturnalStudio

	Brief:
		

	Primary Author:
		 

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"

#define p_ProjMaker NocturnalEngine::ProjectMaker::GetInstance()
namespace NocturnalEngine{
	class ProjectMaker {
	public:
		static ProjectMaker& GetInstance() {
			static ProjectMaker instance;
			return instance;
		}

		bool CreateNewProject(const std::string& name, const std::filesystem::path& basePath,bool setNewAsStartup=true);
		std::string ValidateProjectName(const std::string s);
		bool IsNotAnotherGameDirectory(const std::filesystem::path& projectPath);
	private:
		ProjectMaker() = default;
		void UpdateRootCMake(const std::filesystem::path& projectPath, const std::string& projectName);
		void UpdateProjectFiles(const std::filesystem::path& projectPath, const std::string& projectName);
		void RebuildCmake();
		
		void ReplaceInFile(const std::filesystem::path& filePath,const std::string& search,const std::string& replace);

		std::filesystem::path rootDir{ std::filesystem::current_path().parent_path().parent_path()  };
	};
}
