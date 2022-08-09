cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_GENERATOR "Visual Studio 15 2017")
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

# dependencies of glew, glfw, glad
add_subdirectory("extern/glew-2.1.0/build/cmake")
add_subdirectory("extern/glfw-3.0.4")
set(extern_include_dirs 
  "extern/glad/include"
  "extern/glew-2.1.0/include"
  "extern/glfw-3.0.4/include"
  "./src/include"
)
include_directories(${extern_include_dirs})

find_package(OpenGL)

set(CMAKE_DEBUG_POSTFIX "_d")

if(WIN32)
  set(COMMON_LIBS   glew glfw legacy_stdio_definitions.lib opengl32.lib )
elseif(UNIX)
  find_package(PkgConfig REQUIRED)
  set(COMMON_LIBS glew)
else()
endif()

set(COMMON_LIBS ${COMMON_LIBS} ${EXTRA_LIBS})

set(RUN_DIR ${PROJECT_SOURCE_DIR}/bin)

# find_package(OpenMP)

# if(OPENMP_FOUND)
#   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
# endif()
aux_source_directory(./src lazy_srcs)
set(EXAMPLES )
foreach(src ${lazy_srcs})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${src})
    list(APPEND EXAMPLES ${FILE_NAME})
endforeach(src ${lazy_srcs})



foreach(EXAMPLE ${EXAMPLES})
  add_executable(${EXAMPLE}  src/${EXAMPLE}.cpp "extern/glad/src/glad.c")
  set_property(TARGET ${EXAMPLE} PROPERTY DEBUG_POSTFIX _d)
  target_link_libraries(${EXAMPLE} ${COMMON_LIBS})

  if(MSVC)
    # configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.vcxproj.user @ONLY)
  endif(MSVC)
endforeach(EXAMPLE)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX -std=c++0x")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


ADD_CUSTOM_TARGET(debug ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})
ADD_CUSTOM_TARGET(release ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})

