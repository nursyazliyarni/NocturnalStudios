project(Engine LANGUAGES C CXX)

file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

file(GLOB_RECURSE ENGINE_HDR CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_library(Engine STATIC ${ENGINE_SRC} ${ENGINE_HDR})

target_include_directories(Engine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if (NS_HAVE_GLFW)
  target_link_libraries(Engine PUBLIC glfw)
  target_compile_definitions(Engine PUBLIC USE_GLFW=1)
endif()

if (NS_HAVE_GLEW)
  if (TARGET libglew_static)
    target_link_libraries(Engine PUBLIC libglew_static)
    target_compile_definitions(Engine PUBLIC GLEW_STATIC=1)
  endif()
endif()

if (NS_HAVE_GLM)
  target_link_libraries(Engine PUBLIC glm)
  target_compile_definitions(Engine PUBLIC NS_HAVE_GLM=1)
endif()


if (NS_HAVE_RAPIDJSON)
  target_link_libraries(Engine PUBLIC rapidjson)
endif()

# Silence RapidJSON code-analysis warnings on MSVC only
if (MSVC)
  # treat external headers quietly
  target_compile_options(Engine PRIVATE /external:anglebrackets /external:W0)
  # hush RapidJSON warnings seen via Config.cpp
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/core/Config.cpp
    PROPERTIES COMPILE_OPTIONS "/wd26495;/wd33010"
  )
endif()


if (NS_HAVE_IMGUI)
  target_link_libraries(Engine PUBLIC imgui)
endif()

if (NS_HAVE_STB)
  target_link_libraries(Engine PUBLIC stb)
endif()

# if (NS_HAVE_FREETYPE)
#   target_link_libraries(Engine PUBLIC freetype)
#   target_compile_definitions(Engine PUBLIC NS_HAVE_FREETYPE=1)
# endif()

if (NS_HAVE_FREETYPE)
  target_link_libraries(Engine PUBLIC freetype)
  target_compile_definitions(Engine PUBLIC NS_HAVE_FREETYPE=1)
endif()



if (NS_HAVE_FMOD)
  target_link_libraries(Engine PUBLIC fmod)
endif()

if (WIN32)
  target_link_libraries(Engine PUBLIC opengl32)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${ENGINE_HDR})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source"  PREFIX "Source Files" FILES ${ENGINE_SRC})