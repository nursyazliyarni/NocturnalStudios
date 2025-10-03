project(Sandbox LANGUAGES C CXX)

# grab every subdirectory inside Sandbox (except this CMakeLists itself)
file(GLOB SANDBOX_SUBDIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(subdir ${SANDBOX_SUBDIRS})
  if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt)
      message(STATUS "Adding Sandbox game: ${subdir}")
      add_subdirectory(${subdir})
    endif()
  endif()
endforeach()
