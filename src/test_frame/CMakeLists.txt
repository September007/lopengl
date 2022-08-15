
project(test_frame)

# # copy files to target path
# execute_process(COMMAND powershell ls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glsl  RESULT_VARIABLE  result OUTPUT_QUIET ERROR_QUIET )
# if(result EQUAL 1)
# file(COPY ${CMAKE_CURRENT_LIST_DIR}/glsl  DESTINATION  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# execute_process(COMMAND powershell rm -Force -Recurse ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glsl/HANDSOUT/.git )
# endif()
set(glsl_path "${CMAKE_SOURCE_DIR}/src/test_frame/glsl" CACHE STRING "")
set(glsl_target_path "${CMAKE_SOURCE_DIR}/bin/glsl" CACHE STRING "")
file(GLOB_RECURSE shader_srcs ${glsl_path}/*.glsl)


function(add_shaders_deps target)
    add_custom_command(TARGET ${target}
        POST_BUILD
        COMMAND powershell "if(Test-Path ${glsl_target_path}) { rm -Force -Recurse ${glsl_target_path} }"
        COMMAND powershell cp -Recurse ${glsl_path} "${glsl_target_path}/.."
        COMMAND powershell rm -Force -Recurse ${glsl_target_path}/HANDSOUT/.git
        DEPENDS ${shader_srcs}
    )
endfunction()

# targets 
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/*.cpp test_srcs)
message("currrent_list_dir:${CMAKE_CURRENT_LIST_DIR}")
# actually this is src/test_frame/test_main.cpp
file(GLOB_RECURSE test_main_file test_main.cpp)
message("test_main_file:${test_main_file}")
foreach(src ${test_srcs})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${src})
    message("src:${src}")
    if(${src} EQUAL ${test_main_file})
        continue()
    endif()
    set_property(SOURCE ${FILE_NAME} PROPERTY srcFile APPEND ${test_main_file} )
    set_property(SOURCE ${FILE_NAME} PROPERTY additionalLibs APPEND GTest::gtest )
    
endforeach()

