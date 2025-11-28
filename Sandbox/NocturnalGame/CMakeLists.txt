project(NocturnalGame LANGUAGES C CXX)

file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)
file(GLOB_RECURSE GAME_HDR CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"   "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_executable(NocturnalGame ${GAME_SRC} ${GAME_HDR})
target_link_libraries(NocturnalGame PRIVATE Engine)

if (MSVC)
  target_compile_options(NocturnalGame PRIVATE /W4 /MP)

  # Release → GUI app (no console), still uses int main(...)
  target_link_options(NocturnalGame PRIVATE
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
  )
endif()


# Copy the correct FMOD runtime DLL next to the EXE after build
if (NS_HAVE_FMOD)
  add_custom_command(TARGET NocturnalGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<IF:$<CONFIG:Debug>,${FMOD_DLL_DEBUG},${FMOD_DLL_RELEASE}>
            $<TARGET_FILE_DIR:NocturnalGame>
  )
endif()


# copy assets folder next to the exe 
# Assets live next to the NocturnalGame source, not at project root
set(GAME_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

if (EXISTS "${GAME_ASSETS_DIR}")
  add_custom_command(TARGET NocturnalGame POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${GAME_ASSETS_DIR}"
            "$<TARGET_FILE_DIR:NocturnalGame>/assets"
    COMMENT "Copying NocturnalGame assets to output directory"
  )
endif()


source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${GAME_SRC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${GAME_HDR})

set_target_properties(NocturnalGame PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)




source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${GAME_SRC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${GAME_HDR})

set_target_properties(NocturnalGame PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
if (NS_HAVE_FMOD)
  add_custom_command(TARGET NocturnalGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<IF:$<CONFIG:Debug>,${FMOD_DLL_DEBUG},${FMOD_DLL_RELEASE}>
            $<TARGET_FILE_DIR:NocturnalGame>
  )
endif()




source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${GAME_SRC})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${GAME_HDR})

set_target_properties(NocturnalGame PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)