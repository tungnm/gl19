#pragma once
#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <gtc/type_ptr.hpp>

using std::unordered_map;
using std::string;

class ShaderProgram
{

public:
    ShaderProgram();

    ~ShaderProgram();

    std::string GetProgramName();

    /*
     * fileName: file name of the shader file inside the shaders folder
     * shaderType: either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     */
    bool LoadShader(string shaderName, string fileName, GLuint shaderType);

    /*
     * programName: A name for the program
     */
    bool CreateProgram(string vertexShaderName, string fragmentShaderName);

    GLuint GetProgramHandle();

    void MakeCurrent();

    void SendVec3Uniform(
        std::string uniformName,
        glm::vec3 value);
    void SendVec4Uniform(
        std::string uniformName,
        glm::vec4 value);
    void SendMat4Uniform(
        std::string uniformName,
        glm::mat4 value);
    void SendMat3Uniform(
        std::string uniformName,
        glm::mat3 value);

private:

    string mShaderFolderPath;
    // == vertName + fragName
    string mProgramName;
    GLuint mProgramHandle;
    unordered_map<string, GLuint> mShaderHandleMap;

};