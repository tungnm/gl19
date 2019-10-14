
# add path to third party libraries include files and lib file for linking
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/glfw/include/GLFW")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/glm")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/glad/include")
include_directories("${PROJECT_SOURCE_DIR}/ThirdParty/assimp/include")
link_directories("${PROJECT_SOURCE_DIR}/ThirdParty/glfw/lib-vc2019")
link_directories("${PROJECT_SOURCE_DIR}/ThirdParty/assimp/lib/Release")

set (SOURCES
    src/main.cpp
    src/util.h
    src/util.cpp
    src/ShaderLoader.h
    src/ShaderLoader.cpp
    src/RenderMesh.h
    src/RenderMesh.cpp
    src/Texture.h
    src/Texture.cpp
    src/Scene.h
    src/Scene.cpp
    src/Object.h
    src/Object.cpp
    src/Painter.h
    src/Painter.cpp
    ${PROJECT_SOURCE_DIR}/ThirdParty/glad/src/glad.c
    )

# set the build output folder. In this case openGLTest.exe compiled above will be output to this folder
# Note: need to call this before add_executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# add exe file, compiled by all the sources and header files listed above
# this is a target. A target could be an executable, a library or archive
add_executable (openGLTest ${SOURCES})

# use this .lib files for linking during compilation. The path to these file are specified above 
# with include_directories and link_directories
target_link_libraries(openGLTest glfw3 assimp-vc140-mt)


# This is only for Visual studio on Windows: When clicking the debug button, Visual studio
# will run the executable from inside a folder specified by VS_DEBUGGER_WORKING_DIRECTORY.
# By default it's the local ./Debug folder. Need to set this to the output folder specified above
# so that the executable can access all the dll, shader, models..etc files
# Note the $<CONFIGURATION> variable here is either Debug or Release
set_property(TARGET openGLTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)


# copy the runtime .dll files of the third party library to the output folder 
add_custom_command(TARGET openGLTest POST_BUILD        # Adds a post-build event to openGLTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different      # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/ThirdParty/assimp/bin/Release/assimp-vc140-mt.dll"      # <--this is in-file
       ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)     

# copy the shader folder and model folder to the ouput folder
install (DIRECTORY shaders/ DESTINATION ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/shaders/)
install (DIRECTORY models/ DESTINATION ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/models/)
install (DIRECTORY textures/ DESTINATION ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>/textures/)
