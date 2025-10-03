# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/glfw/CMakeLists.txt)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/glfw)
  set(NS_HAVE_GLFW ON)
endif()

# GLEW
set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_UTILS              OFF CACHE BOOL "" FORCE)
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/glew/CMakeLists.txt)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/glew ${CMAKE_BINARY_DIR}/lib/glew)
  set(NS_HAVE_GLEW ON)
endif()

# GLM (header-only)
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/glm/glm/glm.hpp)
  add_library(glm INTERFACE)
  # NOTE: point to the folder that contains the inner "glm/" directory
  target_include_directories(glm INTERFACE ${CMAKE_SOURCE_DIR}/lib/glm)
  set(NS_HAVE_GLM ON)
endif()


# RapidJSON (header-only)
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/rapidjson/include/rapidjson)
  add_library(rapidjson INTERFACE)
  target_include_directories(rapidjson INTERFACE ${CMAKE_SOURCE_DIR}/lib/rapidjson/include)
  set(NS_HAVE_RAPIDJSON ON)
endif()

# ImGui
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/imgui/CMakeLists.txt)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/imgui)
  set(NS_HAVE_IMGUI ON)
endif()

# stb (header-only)
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/stb/stb_image.h)
  add_library(stb INTERFACE)
  target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/lib/stb)
  set(NS_HAVE_STB ON)
endif()