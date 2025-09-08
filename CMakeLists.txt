cmake_minimum_required(VERSION 3.20)
project(NocturnalStudios LANGUAGES C CXX)

#  Toolchain / warnings 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # prefer static everywhere

if (MSVC)
  add_compile_options(/W4 /permissive- /MP)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#  GLFW (build from source) 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/glfw)

#  GLEW as a subproject (like your screenshot) 
set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)  # <- static-only
set(BUILD_UTILS              OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/glew ${CMAKE_BINARY_DIR}/lib/glew)

# Put libs into build/lib and DLLs/exes into build/bin (for all configs)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(tgt IN ITEMS libglew_static glew_s glew_shared glew)
    if(TARGET ${tgt})
      foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
        set_target_properties(${tgt} PROPERTIES
          ARCHIVE_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/lib
          LIBRARY_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin
          RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin)
      endforeach()
    endif()
  endforeach()
else()
  foreach(tgt IN ITEMS libglew_static glew_s glew_shared glew)
    if(TARGET ${tgt})
      set_target_properties(${tgt} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    endif()
  endforeach()
endif()

#  Your sources
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/source/*.cpp")
file(GLOB_RECURSE HDR CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.h")

if (NOT SRC)
  message(FATAL_ERROR "No sources found in source/. Add at least source/main.cpp")
endif()

add_executable(NocturnalEngine ${SRC} ${HDR})

# After: add_executable(NocturnalEngine ${SRC})

# Make NocturnalEngine the default Startup Project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT NocturnalEngine)

# (Nice-to-have) Ensure F5 runs from the right working dir and with args/env if you need them
set_target_properties(NocturnalEngine PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:NocturnalEngine>"
  # VS_DEBUGGER_COMMAND_ARGUMENTS "--your-args-here"
  # VS_DEBUGGER_ENVIRONMENT "MY_VAR=1;OTHER=foo"
)

# Force exe into build/bin (no Debug/Release folders)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(cfg IN LISTS CMAKE_CONFIGURATION_TYPES)
    set_target_properties(NocturnalEngine PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin)
  endforeach()
else()
  set_target_properties(NocturnalEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# VS Solution Explorer: mirror folder structure
source_group(TREE "${CMAKE_SOURCE_DIR}/source" PREFIX "Source Files" FILES ${SRC})
source_group(TREE "${CMAKE_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HDR})

#  Includes 
target_include_directories(NocturnalEngine PRIVATE
  ${CMAKE_SOURCE_DIR}/source
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/lib/glm
  ${CMAKE_SOURCE_DIR}/lib/rapidjson/include
)


if (MSVC)
  target_compile_options(NocturnalEngine PRIVATE
    /external:anglebrackets
    /external:W0
    /external:I${CMAKE_SOURCE_DIR}/lib/rapidjson/include
  )
endif()


#  Linkage
if (MSVC)
  target_link_libraries(NocturnalEngine PRIVATE opengl32)
endif()
target_link_libraries(NocturnalEngine PRIVATE glfw)

# Prefer static if present; otherwise use shared
if (TARGET libglew_static)
  target_link_libraries(NocturnalEngine PRIVATE libglew_static)
  target_compile_definitions(NocturnalEngine PRIVATE GLEW_STATIC)
elseif (TARGET glew_s)
  target_link_libraries(NocturnalEngine PRIVATE glew_s)
  target_compile_definitions(NocturnalEngine PRIVATE GLEW_STATIC)
elseif (TARGET glew_shared)
  target_link_libraries(NocturnalEngine PRIVATE glew_shared)
elseif (TARGET glew)
  target_link_libraries(NocturnalEngine PRIVATE glew)
else()
  message(FATAL_ERROR "No GLEW target exported by subproject.")
endif()

# Output + assets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (EXISTS "${CMAKE_SOURCE_DIR}/assets")
  add_custom_command(TARGET NocturnalEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:NocturnalEngine>/assets")
endif()

# Messages
message(STATUS "GLFW: using subdirectory at lib/glfw")
message(STATUS "GLEW: subproject binary dir -> ${CMAKE_BINARY_DIR}/lib/glew")
message(STATUS "Outputs: exe -> build/bin, libs -> build/lib")
