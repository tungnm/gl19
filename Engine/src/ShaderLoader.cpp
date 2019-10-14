#pragma once

#include "ShaderLoader.h"
#include <fstream>
#include <iostream>

bool ShaderProgram::LoadShader(string shaderName, string fileName, GLuint shaderType)
{
    if (shaderType != GL_VERTEX_SHADER && shaderType != GL_FRAGMENT_SHADER)
    {
        std::cerr << "Invalid shader type. Must be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER\n";
        return false;
    }

    string shaderPath = mShaderFolderPath + "\\" + fileName;
    std::ifstream ifs(shaderPath);
    if (ifs.fail())
    {
        std::cerr << "Shader file not found: " << shaderPath << std::endl;
        return false;
    }

    std::string shaderSource((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));

    GLuint shaderHandle = glCreateShader(shaderType);
    if (shaderHandle == 0) {
        std::cerr << "Error creating vertex shader";
        return false;
    }

    const GLchar* source[] = { shaderSource.c_str() };

    glShaderSource(shaderHandle, 1, source, nullptr);

    glCompileShader(shaderHandle);

    GLint compilationResultCode;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compilationResultCode);

    if (compilationResultCode == GL_FALSE) {
        std::cerr << shaderPath << ": Shader compilation failed\n";

        GLint logLength;
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {

            std::string log(logLength, ' ');
            GLsizei written;
            glGetShaderInfoLog(shaderHandle, logLength, &written, &log[0]);
            std::cerr << "Compile log: " << log << std::endl;
        }
        else
        {
            std::cerr << "Unable to retrieve shader compilation log\n";
        }

        return false;
    }

    mShaderHandleMap[shaderName] = shaderHandle;

    return true;
}

bool ShaderProgram::CreateProgram(string vertexShaderName, string fragmentShaderName)
{
    GLuint programHandle = glCreateProgram();

    if (!programHandle)
    {
        std::cerr << "Failed to create shader program\n";
        return false;
    }

    if (mShaderHandleMap.count(vertexShaderName) == 0 ||
        mShaderHandleMap.count(fragmentShaderName) == 0)
    {
        std::cerr << "Vertex shader or fragment shader provided not found\n";
        return false;
    }

    glAttachShader(programHandle, mShaderHandleMap[vertexShaderName]);
    glAttachShader(programHandle, mShaderHandleMap[fragmentShaderName]);

    glLinkProgram(programHandle);

    GLint status;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        std::cerr << "Failed to link shader program\n";

        GLint logLength;
        glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0)
        {
            std::string log(logLength, ' ');

            GLsizei written;
            glGetProgramInfoLog(programHandle, logLength, &written, &log[0]);

            std::cerr << "Shader program log: \n" << log;
            return false;
        }
    }
    else
    {
        glUseProgram(programHandle);
        
        mProgramHandle = programHandle;

        glDetachShader(programHandle, mShaderHandleMap[vertexShaderName]);
        glDetachShader(programHandle, mShaderHandleMap[fragmentShaderName]);

    }

    return true;
}

GLuint ShaderProgram::GetProgramHandle()
{
    return mProgramHandle;
}

void ShaderProgram::MakeCurrent()
{
    glUseProgram(mProgramHandle);
}

ShaderProgram::ShaderProgram()
{
    // local path to shader folder
    mShaderFolderPath = "shaders";
}

ShaderProgram::~ShaderProgram()
{
    for (auto item : mShaderHandleMap)
    {
        std::cout << "\nDelete shader: " << item.first << std::endl;
        glDeleteShader(item.second);
    }
    

    std::cout << "\nDelete shader program: " << mProgramHandle << std::endl;
   glDeleteProgram(mProgramHandle);

}