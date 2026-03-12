project(Sandbox LANGUAGES C CXX)

function(add_sandbox_projects dir)
    file(GLOB children RELATIVE ${dir} ${dir}/*)

    foreach(child ${children})
        set(child_path "${dir}/${child}")
        if(IS_DIRECTORY ${child_path})
            if(EXISTS "${child_path}/CMakeLists.txt")
                message(STATUS "Adding Sandbox game: ${child}")
                add_subdirectory(${child_path})
            else()
                # recurse into subdirectory
                add_sandbox_projects(${child_path})
            endif()
        endif()
    endforeach()
endfunction()

# start from Sandbox root
add_sandbox_projects(${CMAKE_CURRENT_SOURCE_DIR})
