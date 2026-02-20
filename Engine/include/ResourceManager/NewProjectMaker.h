/*!------------------------------------------------------------------------------------------------
	File: NewProjectMaker.h
	Project: NocturnalStudio

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
	private:
		ProjectMaker() = default;
		void UpdateRootCMake(const std::filesystem::path& projectPath, const std::string& projectName);
		void UpdateProjectFiles(const std::filesystem::path& projectPath, const std::string& projectName);

		std::filesystem::path templatePath{ std::filesystem::current_path().parent_path().parent_path() / "Resources" / "GameTemplate" };
	};
}
