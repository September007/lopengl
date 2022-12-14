cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_WITH_INSTALL_RPATH )
project(lopengl)

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)
# message("CMAKE_BINARY_DIR:${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Second, for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
###############################################################################################
##################################  3rd dependencies  #########################################
###############################################################################################
# dependencies of glew, glfw, glad
add_subdirectory("extern/glew-2.1.0/build/cmake")
 add_subdirectory("extern/glfw-3.3.8")
add_subdirectory("extern/googletest")
add_subdirectory("extern/fmt" EXCLUDE_FROM_ALL)
#add_subdirectory("src/test_frame")
include("src/test_frame/CMakeLists.txt")
set(extern_include_dirs 
 # "extern/imgui/examples/libs/glfw/include"
  "extern/glad/include"
  "extern/glew-2.1.0/include"
  "extern/fmt/include"
  # "extern/glfw-3.0.4/include"
  "extern/googletest/googletest/include"
  "extern/imgui"
  "src/include"
  # write include dir here , will cause all the target binding this include_dir
  # this is only for the vscode intellisense to judge include dirs
  #"src/test_frame/include"
  "src/include/Light"
)
include_directories(${extern_include_dirs})
include_directories(
  "extern/googletest/googletest/include"
  )
  


find_package(OpenGL)

set(CMAKE_DEBUG_POSTFIX "_d")


if(WIN32)
  set(COMMON_LIBS   glew glfw legacy_stdio_definitions.lib opengl32.lib  fmt::fmt-header-only)
elseif(UNIX)
  find_package(PkgConfig REQUIRED)
  set(COMMON_LIBS glew glfw X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL  fmt::fmt-header-only)
else()
endif()


###############################################################################################
########################  common compiler setting  ############################################
###############################################################################################

if("${CMAKE_BUILD_TYPE}" EQUAL "Debug")
  add_compile_definitions(Debug)
endif()

if(MSVC)
  add_compile_options( /W4 /WX /wd4819)
  target_compile_options(glew PUBLIC /W1)
else()
  add_compile_options(  -Wall -Wextra -pedantic -Werror)
  target_compile_options(glew PUBLIC -w)
endif()


###############################################################################################
##################################  targets  ##################################################
###############################################################################################
file(GLOB_RECURSE lazy_srcs "src/**.cpp")
#  point out programs from which chp need be generate
set(ACTIVED_CHP ch03 ch00 test_frame)
# excluding target
set(exclude_target test_main)
set(EXES )
foreach(src ${lazy_srcs})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${src})
    STRING(REGEX REPLACE ".*/(.+)/(.+)\\..*" "\\1" SUB_FOLDER ${src})
    get_property(oldSrcFile SOURCE ${FILE_NAME}  PROPERTY srcFile)
    set_property(SOURCE ${FILE_NAME} PROPERTY srcFile ${oldSrcFile} ${src} )
    set_property(SOURCE ${FILE_NAME} PROPERTY chpNo ${SUB_FOLDER} )
    list(APPEND EXES ${FILE_NAME})
endforeach(src ${lazy_srcs})

file(GLOB img_srcs "extern/imgui/*.cpp")
list(APPEND img_srcs 
"extern/imgui/backends/imgui_impl_glfw.cpp"
"extern/imgui/backends/imgui_impl_opengl3.cpp"
)
set_property(SOURCE test_app APPEND PROPERTY srcFile 
${img_srcs}
)
set_property(SOURCE "34.PriorGL-frameBuffer" PROPERTY shaderDeps 1)
# set_property(SOURCE "test_app" PROPERTY shaderDeps 1)
# add executable object 
# properties  : specification
# srcFile     : source file list
# chpNo       : name of parentFolder,which is chapter name or what, this can be used to determain whether this target should be generate, which control by ${ACTIVED_CHP} 
# shaderDeps  : if this need shader file support( under the Root/bin/glsl ),if need, there will be a post-build progress to move glsl file to target folder
# additionalLibs  : like gtest
# additionalIncludeDirs  : like src/test_frame/include 

function(this_add_executable exe)
    get_property(srcFile SOURCE ${exe} PROPERTY srcFile)
    get_property(chpNo SOURCE ${exe} PROPERTY chpNo)  
    get_property(shaderDeps SOURCE ${exe} PROPERTY shaderDeps) 
    get_property(additionalLibs SOURCE ${exe} PROPERTY additionalLibs)
    get_property(additionalIncludeDirs SOURCE ${exe} PROPERTY additionalIncludeDirs)
    string(FIND "${ACTIVED_CHP}" ${chpNo} build_or_not )
    string(FIND "${exclude_target}" ${exe} exclude_or_not)
    message("t:${exe};chp:${chpNo};buildornot:${build_or_not};srcFs:${srcFile};shDep:${shaderDeps};addlib:${additionalLibs};additionalIncludeDirs:${additionalIncludeDirs}")
    if(${build_or_not} EQUAL -1 OR ${exclude_or_not} EQUAL 0)
        return()
    endif()
    add_executable(${exe} ${srcFile} "extern/glad/src/glad.c" )
    target_link_libraries(${exe} ${COMMON_LIBS} ${additionalLibs})
    if(${shaderDeps} AND ${shaderDeps} EQUAL 1)
        message("target ${exe} will have add_shaders_deps")
        add_shaders_deps(${exe})
    endif()
    if( additionalIncludeDirs)
        message("target ${exe} will have adddirs:${additionalIncludeDirs}")
        target_include_directories(${exe} PUBLIC ${additionalIncludeDirs})
    endif()
endfunction()


foreach(exe ${EXES})
  this_add_executable(${exe})    
endforeach()


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX -std=c++0x")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


ADD_CUSTOM_TARGET(debug ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})
ADD_CUSTOM_TARGET(release ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})


get_property(srcFile SOURCE test_nv12_to_rgb PROPERTY srcFile)
message("outside property srcFile:${srcFile}")