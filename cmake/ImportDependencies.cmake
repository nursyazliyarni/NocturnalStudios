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

# ImGuizmo
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/imguizmo/ImGuizmo.h)
  add_library(ImGuizmo STATIC
    ${CMAKE_SOURCE_DIR}/lib/imguizmo/ImGuizmo.cpp
  )
  target_include_directories(ImGuizmo PUBLIC ${CMAKE_SOURCE_DIR}/lib/imguizmo)
  target_link_libraries(ImGuizmo PUBLIC imgui)
  set(NS_HAVE_IMGUZMO ON)
endif()

# stb (header-only)
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/stb/stb_image.h)
  add_library(stb INTERFACE)
  target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/lib/stb)
  set(NS_HAVE_STB ON)
endif()

# spdlog
if (EXISTS ${CMAKE_SOURCE_DIR}/lib/spdlog/include/spdlog)
  add_library(spdlog INTERFACE)
  target_include_directories(spdlog INTERFACE ${CMAKE_SOURCE_DIR}/lib/spdlog/include)
  set(NS_HAVE_SPDLOG ON)
endif()

# fmod
# FMOD SDK detection + imported target
set(FMOD_ROOT "${CMAKE_SOURCE_DIR}/lib/fmod" CACHE PATH "FMOD SDK root")
set(FMOD_CORE_INC "${FMOD_ROOT}/api/core/inc")
set(FMOD_LIB_X86  "${FMOD_ROOT}/api/core/lib/x86")
set(FMOD_LIB_X64  "${FMOD_ROOT}/api/core/lib/x64")

if(EXISTS "${FMOD_CORE_INC}/fmod.h")
    add_library(fmod UNKNOWN IMPORTED GLOBAL)

    set_target_properties(fmod PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${FMOD_CORE_INC}"
    )

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(FMOD_LIB_DIR "${FMOD_LIB_X64}")
        set(FMOD_IMPORT_LIB_RELEASE "${FMOD_LIB_X64}/fmod_vc.lib")
        set(FMOD_IMPORT_LIB_DEBUG   "${FMOD_LIB_X64}/fmodL_vc.lib")
        set(FMOD_DLL_RELEASE        "${FMOD_LIB_X64}/fmod.dll")
        set(FMOD_DLL_DEBUG          "${FMOD_LIB_X64}/fmodL.dll")
    else()
        set(FMOD_LIB_DIR "${FMOD_LIB_X86}")
        set(FMOD_IMPORT_LIB_RELEASE "${FMOD_LIB_X86}/fmod_vc.lib")
        set(FMOD_IMPORT_LIB_DEBUG   "${FMOD_LIB_X86}/fmodL_vc.lib")
        set(FMOD_DLL_RELEASE        "${FMOD_LIB_X86}/fmod.dll")
        set(FMOD_DLL_DEBUG          "${FMOD_LIB_X86}/fmodL.dll")
    endif()

    if(EXISTS "${FMOD_IMPORT_LIB_RELEASE}")
        set_target_properties(fmod PROPERTIES
            IMPORTED_LOCATION_RELEASE "${FMOD_IMPORT_LIB_RELEASE}"
            IMPORTED_LOCATION_RELWITHDEBINFO "${FMOD_IMPORT_LIB_RELEASE}"
            IMPORTED_LOCATION_MINSIZEREL "${FMOD_IMPORT_LIB_RELEASE}"
        )
    else()
        message(WARNING "FMOD release import lib not found at ${FMOD_IMPORT_LIB_RELEASE}")
    endif()
        if(EXISTS "${FMOD_IMPORT_LIB_DEBUG}")
        set_target_properties(fmod PROPERTIES
            IMPORTED_LOCATION_DEBUG "${FMOD_IMPORT_LIB_DEBUG}"
        )
    else()
        # fallback: use release import lib for Debug if SDK doesn't provide debug import lib
        if(EXISTS "${FMOD_IMPORT_LIB_RELEASE}")
            set_target_properties(fmod PROPERTIES
                IMPORTED_LOCATION_DEBUG "${FMOD_IMPORT_LIB_RELEASE}"
            )
            message(STATUS "FMOD debug import lib not found; using release import lib for Debug")
        else()
            message(WARNING "FMOD debug import lib not found and no release lib to fall back to.")
        endif()
    endif()

    # Expose DLL paths so callers (executable CMakeLists) can copy them to output
    set(FMOD_DLL_RELEASE "${FMOD_DLL_RELEASE}" CACHE FILEPATH "FMOD release DLL")
    set(FMOD_DLL_DEBUG   "${FMOD_DLL_DEBUG}"   CACHE FILEPATH "FMOD debug DLL")

    set(NS_HAVE_FMOD ON)
else()
    message(STATUS "FMOD not found at ${FMOD_CORE_INC}; FMOD disabled")
endif()

if (EXISTS ${CMAKE_SOURCE_DIR}/lib/freetype/CMakeLists.txt)
  add_subdirectory(${CMAKE_SOURCE_DIR}/lib/freetype ${CMAKE_BINARY_DIR}/lib/freetype)
  set(NS_HAVE_FREETYPE ON)
endif()