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
    void PrepareObjectForRender(
        ShaderProgram* shader,
        Object* obj,
        Stage* stage);

    glm::mat4 CalculateMVP(Object* obj, Stage* stage);

    void RenderObject(Object* obj);
    
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
private:
    unsigned int depthMapFBO;
    unsigned int depthMap;
    // shader to render depth map from light view
    ShaderProgram mDepthShader;

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