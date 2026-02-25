/*!------------------------------------------------------------------------------------------------
	File: NewProjectMaker.h
	Game Project: NocturnalStudio

	Brief:


	Primary Author:


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "ResourceManager/NewProjectMaker.h"

namespace NocturnalEngine {
    /*!*************************************************************************
    Function called by LevelEditor once the user has decided where and what they
    they want their new game prokect to be. Creates new file, configures the
    text files in the game to all have the selected game name (must be validated by
    ValidateProjectName) and then if the user wants edits the root cmake file to
    make it so that the next time they open the solution thenew game is set as the
    Startup project
    ****************************************************************************/
    bool ProjectMaker::CreateNewProject(const std::string& name, const std::filesystem::path& basePath, bool setNewAsStartup) {

        std::filesystem::path templatePath = rootDir / "Resources" / "GameTemplate";
        std::filesystem::path projectPath = basePath / name;
      

        try
        {
            // 1. Check if template exists
            if (!std::filesystem::exists(templatePath))
            {
                NOCTURNAL_ENGINE_ERROR("Template project not found at: {}", templatePath.string());
                return false;
            }

            // 2. Create project directory
            if (std::filesystem::exists(projectPath))
            {
                NOCTURNAL_ENGINE_ERROR("Game Project '{}' already exists!", name);
                return false;
            }

            // 3. Copy entire template folder recursively
            std::filesystem::copy(templatePath, projectPath,std::filesystem::copy_options::recursive);


            // 4. Update project-specific files 
            UpdateProjectFiles(projectPath, name);

            // 5. Update root CMakeLists.txt to include new project 

            if(setNewAsStartup)UpdateRootCMake(projectPath, name);

            // 6. Rebuild CMake
            RebuildCmake();

            return true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            NOCTURNAL_ENGINE_ERROR("Failed to create project: {}", e.what());
            return false;
        }
    }

    /*!*************************************************************************
    Updates the CMake at the root directory so that the new file will be the startup
    ****************************************************************************/
    void ProjectMaker::UpdateRootCMake(const std::filesystem::path& projectPath,const std::string& projectName) const
    {
		UNREFERENCED_PARAMETER(projectPath);
        std::filesystem::path rootCMake =rootDir/"CMakeLists.txt";

        if (!std::filesystem::exists(rootCMake))
        {
            NOCTURNAL_ENGINE_WARN("Root CMakeLists.txt not found, skipping update");
            return;
        }

        std::ifstream inFile(rootCMake);
        std::string content((std::istreambuf_iterator<char>(inFile)),
            std::istreambuf_iterator<char>());
        inFile.close();

        // Find the marker where we add new projects
        std::string marker = "set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ";
        size_t pos = content.find(marker);

        if (pos != std::string::npos)
        {

            size_t endPos = content.find(')', pos);
            // Delete everything from marker to end of line
            content.erase(pos, endPos - pos + 1);
            std::string newInsert =marker + projectName+")";

            std::string newLine = marker + projectName + ")";
            content.insert(pos, newLine);

            std::ofstream outFile(rootCMake);
            outFile << content;

            //NOCTURNAL_ENGINE_INFO("Updated startup project to: {}", projectName);
        }
        else
        {
            NOCTURNAL_ENGINE_WARN("Could not find marker '# ADD_NEW_PROJECTS_HERE' in root CMakeLists.txt");
        }
    }

    /*!*************************************************************************
    Iterates through all text files and replaces the string TEMPLATE_PROJECT_NAME
    with the project's name. also adds a default scene
    ****************************************************************************/
    void ProjectMaker::UpdateProjectFiles(const std::filesystem::path& projectPath,const std::string& projectName)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(projectPath))
        {
            if (!entry.is_regular_file())
                continue;

            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return std::tolower(c); });

            // Only modify source/include files
            if (ext == ".cpp" || ext == ".h" || ext == ".hpp" ||
                ext == ".c" || ext == ".json" || ext == ".cmake" ||
                ext == ".txt")
            {
                ReplaceInFile(entry.path(), "TEMPLATE_PROJECT_NAME", projectName);
            }
        }

        // Create a default scene with project name
        std::filesystem::path scenesPath = projectPath / "Assets" / "Scene";
        if (!std::filesystem::exists(scenesPath))
        {
            std::filesystem::create_directories(scenesPath);
        }

        // Create default scene JSON
        std::filesystem::path defaultScene = scenesPath / "DefaultScene.json";
        std::ofstream sceneFile(defaultScene);
        sceneFile << R"({
                        "LayerDrawOrder": [0, 1, 2, 3, 4, 5, 6, 7, 8],
                        "Entities": []
                         })";
        sceneFile.close();
    }

    /*!*************************************************************************
    This makes sure the user's input name is ok (it removes trailing whitespaces
    and replaces all other whitespaces with underscores)
    ****************************************************************************/
    std::string ProjectMaker::ValidateProjectName(const std::string s) {

        std::string newProjName = s;
        for (char& c : newProjName)
        {
            if (c == ' ' || c == '\t' || c == '\n')
            {
                c = '_';
            }
        }
        while (!newProjName.empty()&&newProjName.back() == '_')newProjName.pop_back();
        return newProjName;
    }

    /*!*************************************************************************
    Runs a system call to rebuild the root cmake
    ****************************************************************************/
    void ProjectMaker::RebuildCmake() {
        std::filesystem::path buildDir = rootDir / "build";
        std::string command = "cmake -S " + rootDir.string() + " -B "+buildDir.string();
        system(command.c_str());
    }


    /*!*************************************************************************
    This checks if theres already another cmake file in any of the folders the
    user wants to generate the new game folder in
    ****************************************************************************/
    bool ProjectMaker::IsNotAnotherGameDirectory(const std::filesystem::path& projectPath) const{
        std::filesystem::path checkPath = projectPath;
        while (checkPath!=rootDir/"Sandbox")
        {
            if (std::filesystem::exists(checkPath / "CMakeLists.txt"))return false;
            else checkPath = checkPath.parent_path();
        }
        return true;
    }

    /*!*************************************************************************
    A helper function for UpdateProjectFiles to help replace strings
    opens up the file and replacesall instances of search with replace
    then writes to the file
    ****************************************************************************/
    void ProjectMaker::ReplaceInFile(const std::filesystem::path& filePath, const std::string& search, const std::string& replace)
    {
        std::ifstream inFile(filePath);
        if (!inFile.is_open())
            return;

        std::string content((std::istreambuf_iterator<char>(inFile)),
            std::istreambuf_iterator<char>());
        inFile.close();

        size_t pos = 0;
        while ((pos = content.find(search, pos)) != std::string::npos)
        {
            content.replace(pos, search.length(), replace);
            pos += replace.length();
        }

        std::ofstream outFile(filePath);
        outFile << content;
    }

    
}
