/*!------------------------------------------------------------------------------------------------
	File: NewProjectMaker.h
	Game Project: NocturnalStudio

	Brief:	This file contains the class declaration for a helper class that enables the user
			to make a new game project. This class is used by the leveleditor when making a new
			game project.
		

	Primary Author:
			LAYCOCK ENHUI SARAH (100%)
			- Created the projectmaker class and the related functions
		 

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"

#define p_ProjMaker NocturnalEngine::ProjectMaker::GetInstance()
namespace NocturnalEngine{
	/*!*************************************************************************
	I didn't want to shove all this code into LevelEditor, so i decided to make a
	new class for it. This basically helps the user if they want to make a new
	game project under this engine
	****************************************************************************/
	class ProjectMaker {
	public:
		/*!*************************************************************************
		Singleton, returns a reference
		****************************************************************************/
		static ProjectMaker& GetInstance() {
			static ProjectMaker instance;
			return instance;
		}

		/*!*************************************************************************
		Function called by LevelEditor once the user has decided where and what they
		they want their new game prokect to be. Creates new file, configures the
		text files in the game to all have the selected game name (must be validated by
		ValidateProjectName) and then if the user wants edits the root cmake file to
		make it so that the next time they open the solution thenew game is set as the
		Startup project
		****************************************************************************/
		bool CreateNewProject(const std::string& name, const std::filesystem::path& basePath,bool setNewAsStartup=true);

		/*!*************************************************************************
		This makes sure the user's input name is ok (it removes trailing whitespaces
		and replaces all other whitespaces with underscores)
		****************************************************************************/
		std::string ValidateProjectName(const std::string s);

		/*!*************************************************************************
		This checks if theres already another cmake file in any of the folders the
		user wants to generate the new game folder in
		****************************************************************************/
		bool IsNotAnotherGameDirectory(const std::filesystem::path& projectPath) const;
	private:
		/*!*************************************************************************
		Constructor
		****************************************************************************/
		ProjectMaker() = default;

		/*!*************************************************************************
		Updates the CMake at the root directory so that the new file will be the startup
		****************************************************************************/
		void UpdateRootCMake(const std::filesystem::path& projectPath, const std::string& projectName)const;

		/*!*************************************************************************
		Iterates through all text files and replaces the string TEMPLATE_PROJECT_NAME
		with the project's name
		****************************************************************************/
		void UpdateProjectFiles(const std::filesystem::path& projectPath, const std::string& projectName);

		/*!*************************************************************************
		Runs a system call to rebuild the root cmake
		****************************************************************************/
		void RebuildCmake();
		
		/*!*************************************************************************
		A helper function for UpdateProjectFiles to help replace strings
		****************************************************************************/
		void ReplaceInFile(const std::filesystem::path& filePath,const std::string& search,const std::string& replace);

		//the root directory
		std::filesystem::path rootDir{ std::filesystem::current_path().parent_path().parent_path()  };
	};
}
