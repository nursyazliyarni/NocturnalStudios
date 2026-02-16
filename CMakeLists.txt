cmake_minimum_required(VERSION 3.20)
project(NocturnalStudios LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Utilities)
include(ImportDependencies)

if (MSVC)
    add_compile_options(/W4 /MP)
endif()

add_subdirectory(Engine)
add_subdirectory(Sandbox)


set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT NocturnalGame)
