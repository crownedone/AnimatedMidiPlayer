cmake_minimum_required(FATAL_ERROR VERSION 3.0)

project(myproject)

find_package(Boost REQUIRED COMPONENTS system filesystem) 
find_package(GLEW REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-nativefiledialog CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

include(FetchContent)
find_package(Git REQUIRED)

FetchContent_Populate(
	cxxmidi                             
	GIT_REPOSITORY https://github.com/5tan/cxxmidi.git
	SOURCE_DIR     ../external/cxxmidi              # (Relative) path within in the build directory.
)
 
SET(SOURCE 

${CMAKE_CURRENT_LIST_DIR}/myproj/default_constants.h
${CMAKE_CURRENT_LIST_DIR}/myproj/errors.h
${CMAKE_CURRENT_LIST_DIR}/myproj/helperFunctions.h
${CMAKE_CURRENT_LIST_DIR}/myproj/main.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myCamera.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myCamera.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myFBO.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myFBO.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myLight.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myLight.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myLights.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myLights.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myMaterial.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myMaterial.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myObject.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myObject.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myScene.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myScene.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myShader.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myShader.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myShaders.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myShaders.h
${CMAKE_CURRENT_LIST_DIR}/myproj/mySubObject.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/mySubObject.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myTexture.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myTexture.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myVAO.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myVAO.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myVBO.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/myVBO.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myWaterFBOs.h
${CMAKE_CURRENT_LIST_DIR}/myproj/myWaterFBOs.cpp
${CMAKE_CURRENT_LIST_DIR}/myproj/mySkybox.h
${CMAKE_CURRENT_LIST_DIR}/myproj/mySkybox.cpp
)
 
 
add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::nativefiledialog::nfd)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW) 
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::system Boost::filesystem) 

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm.lib)
elseif(UNIX)
    find_package(ALSA REQUIRED)
    target_include_directories(PROJECT_NAME PRIVATE ${ALSA_INCLUDE_DIRS})                  
    target_link_libraries (${PROJECT_NAME} PRIVATE ${ALSA_LIBRARIES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external/cxxmidi/include) #include downloaded cxxmidi

add_custom_command(
         TARGET ${PROJECT_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
             ${CMAKE_CURRENT_SOURCE_DIR}/myproj/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
         COMMENT "Copying shaders" VERBATIM
         )
add_custom_command(
         TARGET ${PROJECT_NAME} POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
             ${CMAKE_CURRENT_SOURCE_DIR}/myproj/models ${CMAKE_CURRENT_BINARY_DIR}/models
         COMMENT "Copying models" VERBATIM
         )