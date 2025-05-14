cmake_minimum_required (VERSION 3.0.2)
# allows access to environment variables with the name 
project (InCGFramework)

# Project Structure::
# InCG_Parent_Folder       
#	|--a01
#	|   |-- a01.cpp, a01.hpp
#	|	|-- shader
#	|	|	| -- gbuffer.vs.glsl, gbuffer.fs.glsl, ...
#	|	|-- data
#	|	|	| -- teapot.obj, ...
#	|	|-- cmake
#	|	|	| -- FindExternal.cmake
#	|	|-- CMakeLists.txt
#	|	|-- external     # (Optional)
#	|	|	|-- AntTweakBar, assimp, glbinding-2.1.4, glfw-3.1.1, incg, glm-0.9.9.5
#	|--a02, a03, a04, ...   # similar to a01
#	|--external     # (Optional)
#	|	|-- AntTweakBar, assimp, glbinding-2.1.4, glfw-3.1.1, incg, glm-0.9.9.5
#
# Either a copy of the external libs folder is located in each assignment Folder: aXX 
# Or a copy of the external libs folder is located in the Parent_Folder
# Or the libs are installed  (and located by FindExternal)

# external librarys found in parent folder
set(LOCAL_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external)
if(NOT EXISTS ${LOCAL_EXTERNAL_DIR})
	# if external librarys are not found in parent folder, they can be in the assignment folder
 	set(LOCAL_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
endif()


list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(FindExternal)
# set global dependency dir for find_external calls
if (EXISTS "/soft/devtools/incg-2019")
	set(EXTERNALS_DIR "/soft/devtools/incg-2019" CACHE STRING "Path containing dependencies.")
else()
	set(EXTERNALS_DIR "/proj/i9incg/2019ss" CACHE STRING "Path containing dependencies.")
endif()


# installation
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
#before adding libraries, define the output paths
# MSVC & Xcode automatically create the build-type folders
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}")
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}")

# set binary location, because of out-of-tree builds
set(LOCAL_EXTERNAL_BIN_DIR "${PROJECT_BINARY_DIR}/external")



# add glbindings
option(OPTION_BUILD_TOOLS		OFF)
option(OPTION_BUILD_TESTS		OFF)
option(OPTION_BUILD_GPU_TESTS	OFF)
option(BUILD_SHARED_LIBS		OFF)
add_subdirectory(${LOCAL_EXTERNAL_DIR}/glbinding-2.1.4 ${LOCAL_EXTERNAL_BIN_DIR}/glbinding)
include_directories(SYSTEM ${LOCAL_EXTERNAL_DIR}/glbinding-2.1.4/source/glbinding/include)
# hide glbinding options
mark_as_advanced(OPTION_BUILD_TESTS OPTION_BUILD_TOOLS OPTION_BUILD_GPU_TESTS
 OPTION_BUILD_WITH_BOOST_THREAD OPTION_SELF_CONTAINED OPTION_BUILD_DOCS
 OPTION_BUILD_EXAMPLES PYTHON)

# add glfw
# configure glfw building
option(GLFW_BUILD_DOCS     OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS    OFF)
option(GLFW_INSTALL        OFF)
option(BUILD_SHARED_LIBS   OFF)
# add glfw build system
add_subdirectory(${LOCAL_EXTERNAL_DIR}/glfw-3.3.4 ${LOCAL_EXTERNAL_BIN_DIR}/glfw)
# include glfw headers
include_directories(SYSTEM ${LOCAL_EXTERNAL_DIR}/glfw-3.3.4/include)
# hide glfw options
mark_as_advanced(GLFW_BUILD_DOCS GLFW_BUILD_TESTS GLFW_INSTALL GLFW_BUILD_EXAMPLES
 GLFW_DOCUMENT_INTERNALS GLFW_USE_EGL GLFW_USE_MIR GLFW_USE_WAYLAND GLFW_LIBRARIES
 LIB_SUFFIX)

# include glm
include_directories(SYSTEM ${LOCAL_EXTERNAL_DIR}/glm-0.9.9.8)




# add incg
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${LOCAL_EXTERNAL_DIR}/incg ${LOCAL_EXTERNAL_BIN_DIR}/incg)
include_directories(SYSTEM ${LOCAL_EXTERNAL_DIR}/incg/include)




# activate C++ 11
if(NOT MSVC)
    add_definitions(-std=c++11)
    # all warnings, implicit lossy casts
    add_definitions(-Wall -Wconversion -Wextra -Wpedantic)
    # warn when members are unitialized
    add_definitions(-Weffc++)
    # force linking  with c++11 lib
    if(APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
        add_definitions(-stdlib=libc++)
    endif()
else()
    # build in parallel, show warnings and suppress one caused by glbinding
    add_definitions(/MP /W3 /wd4251)
endif()

if(NOT MSVC)
    add_definitions(-Wall -Wextra -Wpedantic -Wconversion -Wnon-virtual-dtor -Wno-missing-field-initializers -Wno-unused-function)
endif()


#before adding executable, set output path
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}")
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.c" "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.cxx")

get_filename_component(FOLDER_NAME "${CMAKE_SOURCE_DIR}" NAME)

add_executable(${FOLDER_NAME} ${SOURCES})
target_link_libraries(${FOLDER_NAME} glfw glbinding incg)
target_include_directories(${FOLDER_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
set_property(TARGET ${FOLDER_NAME} PROPERTY CXX_STANDARD 11)
