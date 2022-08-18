
#project(test_frame)

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
aux_source_directory(${CMAKE_CURRENT_LIST_DIR} test_srcs)
message("currrent_list_dir:${CMAKE_CURRENT_LIST_DIR}")
# actually this is src/test_frame/test_main.cpp
file(GLOB_RECURSE test_main_file test_main.cpp)
message("test_main_file:${test_main_file}")
message("test_srcs:${test_srcs}")
foreach(src ${test_srcs})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${src})
    if(NOT ${src}  EQUAL ${test_main_file})
        get_property(oldSrcFile SOURCE ${FILE_NAME} PROPERTY srcFile )
        get_property(oldAdditionalLibs SOURCE ${FILE_NAME} PROPERTY additionalLibs )
        #message("here ${FILE_NAME} oldSrcFile:${oldSrcFile};addlib:${oldAdditionalLibs}")
        set_property(SOURCE ${FILE_NAME} PROPERTY srcFile ${oldSrcFile} ${test_main_file} )
        set_property(SOURCE ${FILE_NAME} PROPERTY additionalLibs ${oldAdditionalLibs}  GTest::gtest )
        set_property(SOURCE ${FILE_NAME} PROPERTY additionalIncludeDirs ${CMAKE_SOURCE_DIR}/src/test_frame/include)
        
    endif()
endforeach()

get_property(srcFile SOURCE test_nv12_to_rgb PROPERTY srcFile)
message("inside property srcFile:${srcFile}")