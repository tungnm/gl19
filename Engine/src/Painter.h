#pragma once

#include "Object.h"
#include <vector>
#include "ShaderLoader.h"
class Painter
{
private:
    void BindTextureIfExist(std::string textureNameInShader, Texture* texture);
protected:

    // Common painter functionalities:

    // This method calculate object's MV, MVP, normalMatrix
    // and send them to the shader. It also bind the object
    // VAO. It also bind texture channel if available in the object Material
    void PrepareObjectForRender(Object* obj);

    void RenderObject(Object* obj);

    

    void SendVec3UniformToShader(
        std::string uniformName,
        glm::vec3 value);
    void SendVec4UniformToShader(
        std::string uniformName,
        glm::vec4 value);
    void SendMat4UniformToShader(
        std::string uniformName,
        glm::mat4 value);
    void SendMat3UniformToShader(
        std::string uniformName,
        glm::mat3 value);


    std::vector<Object* > mObjects;
    Stage* mStage;
    ShaderProgram mShaderProgram;

public:
    void AssignObjects(Object* object);
    void AssignStage(Stage* stage);
    
    virtual void DrawObjects() = 0;

    // should load shader here
    virtual void Init() = 0;

    Painter(Stage* stage = nullptr);
    ~Painter();
};

class GouraudPainter : public Painter
{
public:
    void DrawObjects();
    void Init();

    GouraudPainter();
    ~GouraudPainter();
};

class PhongNormalMapPainter : public Painter
{
public:
    void DrawObjects();
    void Init();

    PhongNormalMapPainter();
    ~PhongNormalMapPainter();
};