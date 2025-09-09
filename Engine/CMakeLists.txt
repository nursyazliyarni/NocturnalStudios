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

if (NS_HAVE_RAPIDJSON)
  target_link_libraries(Engine PUBLIC rapidjson)
endif()

if (NS_HAVE_IMGUI)
  target_link_libraries(Engine PUBLIC imgui)
endif()

if (NS_HAVE_STB)
  target_link_libraries(Engine PUBLIC stb)
endif()

if (WIN32)
  target_link_libraries(Engine PUBLIC opengl32)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${ENGINE_HDR})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/source"  PREFIX "Source Files" FILES ${ENGINE_SRC})
