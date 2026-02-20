project(GartenOfBanBan300 LANGUAGES C CXX)

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
add_executable(GartenOfBanBan300 ${GAME_SRC} ${GAME_HDR})

# Include directories for game headers
target_include_directories(GartenOfBanBan300 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

# Link engine library
target_link_libraries(GartenOfBanBan300 PRIVATE Engine)

# Compile definitions
target_compile_definitions(GartenOfBanBan300 PUBLIC
  $<$<CONFIG:Debug>:NOCTURNAL_PLATFORM_WINDOWS;_DEBUG;_CONSOLE>
  $<$<CONFIG:Release>:NOCTURNAL_PLATFORM_WINDOWS;NDEBUG;_CONSOLE>
)

# MSVC specific settings
if (MSVC)
  target_compile_options(GartenOfBanBan300 PRIVATE /W4 /MP)
  target_link_options(GartenOfBanBan300 PRIVATE
    "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
    "$<$<CONFIG:Release>:/ENTRY:mainCRTStartup>"
  )
endif()

# Copy FMOD DLL (only once, removed duplicate)
if (NS_HAVE_FMOD)
  add_custom_command(TARGET GartenOfBanBan300 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<IF:$<CONFIG:Debug>,${FMOD_DLL_DEBUG},${FMOD_DLL_RELEASE}>
            $<TARGET_FILE_DIR:GartenOfBanBan300>
  )
endif()

# Visual Studio folder organization
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${GAME_SRC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${GAME_HDR})

# Set working directory for debugging
set_target_properties(GartenOfBanBan300 PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)