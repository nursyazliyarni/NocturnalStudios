project(TEMPLATE_PROJECT_NAME LANGUAGES C CXX)

# Collect source files
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cxx"
)

# Collect header files
file(GLOB_RECURSE GAME_HDR CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"
)

# Create executable
add_executable(TEMPLATE_PROJECT_NAME ${GAME_SRC} ${GAME_HDR})

# Include directories for game headers
target_include_directories(TEMPLATE_PROJECT_NAME PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

# Link engine library
target_link_libraries(TEMPLATE_PROJECT_NAME PRIVATE Engine)

# Compile definitions
target_compile_definitions(TEMPLATE_PROJECT_NAME PUBLIC
  $<$<CONFIG:Debug>:NOCTURNAL_PLATFORM_WINDOWS;_DEBUG;_CONSOLE>
  $<$<CONFIG:Release>:NOCTURNAL_PLATFORM_WINDOWS;NDEBUG;_CONSOLE>
)

# MSVC specific settings
if (MSVC)
  target_compile_options(TEMPLATE_PROJECT_NAME PRIVATE /W4 /MP)
  target_link_options(TEMPLATE_PROJECT_NAME PRIVATE
    "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
    "$<$<CONFIG:Release>:/ENTRY:mainCRTStartup>"
  )
endif()

# Copy FMOD DLL (only once, removed duplicate)
if (NS_HAVE_FMOD)
  add_custom_command(TARGET TEMPLATE_PROJECT_NAME POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<IF:$<CONFIG:Debug>,${FMOD_DLL_DEBUG},${FMOD_DLL_RELEASE}>
            $<TARGET_FILE_DIR:TEMPLATE_PROJECT_NAME>
  )
endif()

# Visual Studio folder organization
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${GAME_SRC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${GAME_HDR})

# Set working directory for debugging
set_target_properties(TEMPLATE_PROJECT_NAME PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)