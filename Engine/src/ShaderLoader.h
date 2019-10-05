#pragma once
#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

class ShaderManager
{

public:
    ShaderManager();

    ~ShaderManager();

    /*
     * fileName: file name of the shader file inside the shaders folder
     * shaderType: either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     */
    bool LoadShader(string shaderName, string fileName, GLuint shaderType);

    /*
     * programName: A name for the program
     */
    bool CreateProgram(string programName, string vertexShaderName, string fragmentShaderName);

    GLuint GetProgramHandle(string programName);
private:

    string mShaderFolderPath;

    unordered_map<string, GLuint> mShaderHandleMap;
    unordered_map<string, GLuint> mProgramHandleMap;

};