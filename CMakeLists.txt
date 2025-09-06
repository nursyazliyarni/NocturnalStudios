cmake_minimum_required(VERSION 3.20)
project(NocturnalStudios CXX)

# C++ Standard + warnings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
  add_compile_options(/W4 /permissive- /MP)  # strict warnings, multiproc compile
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#  Prefer static libs (avoid DLL issues) 
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# GLFW (vendored, source with CMake) 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/glfw)

# Your sources (auto-pick everything under source/) 
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/source/*.cpp")
add_executable(NocturnalEngine ${SRC})

# Mirror folder structure in Solution Explorer
source_group(TREE "${CMAKE_SOURCE_DIR}/source" PREFIX "Source Files" FILES ${SRC})

# Include paths (GLM is header-only, engine headers in include/) 
target_include_directories(NocturnalEngine PRIVATE
  ${CMAKE_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/include         # engine headers
  ${CMAKE_SOURCE_DIR}/lib/glm
  ${CMAKE_SOURCE_DIR}/lib/glfw/include
  ${CMAKE_SOURCE_DIR}/lib/glew/include
)

# Link OpenGL + GLFW 
if (MSVC)
  target_link_libraries(NocturnalEngine PRIVATE opengl32)
endif()
target_link_libraries(NocturnalEngine PRIVATE glfw)

# GLEW (handles both source repo and prebuilt .lib) 
set(GLEW_Linked FALSE)
if (EXISTS "${CMAKE_SOURCE_DIR}/lib/glew/CMakeLists.txt")
  set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
  set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
  add_compile_definitions(GLEW_STATIC)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/glew)

  if (TARGET libglew_static)
    target_link_libraries(NocturnalEngine PRIVATE libglew_static)
    set(GLEW_Linked TRUE)
  elseif (TARGET glew_s)
    target_link_libraries(NocturnalEngine PRIVATE glew_s)
    set(GLEW_Linked TRUE)
  elseif (TARGET glew) # shared fallback
    target_link_libraries(NocturnalEngine PRIVATE glew)
    set(GLEW_Linked TRUE)
  endif()
endif()

if (NOT GLEW_Linked)
  find_library(GLEW_LIB
    NAMES glew32s glew32 glew
    PATHS ${CMAKE_SOURCE_DIR}/lib/glew ${CMAKE_SOURCE_DIR}/lib/glew/lib
          ${CMAKE_SOURCE_DIR}/lib/glew/lib/Release
          ${CMAKE_SOURCE_DIR}/lib/glew/lib/Debug
  )
  if (GLEW_LIB)
    get_filename_component(_glew_name "${GLEW_LIB}" NAME_WE)
    if (_glew_name MATCHES "glew32s|glew_s|glewstatic")
      add_compile_definitions(GLEW_STATIC)
    endif()
    target_link_libraries(NocturnalEngine PRIVATE "${GLEW_LIB}")
    set(GLEW_Linked TRUE)
  endif()
endif()

if (NOT GLEW_Linked)
  message(FATAL_ERROR "GLEW not found. Keep lib/glew (source with CMake OR prebuilt .lib + include).")
endif()

# Output exe in build/bin 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy assets/ next to exe (optional) 
if (EXISTS "${CMAKE_SOURCE_DIR}/assets")
  add_custom_command(TARGET NocturnalEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:NocturnalEngine>/assets)
endif()