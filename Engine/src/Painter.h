#pragma once

#include "Object.h"
#include <vector>
#include "ShaderLoader.h"
class Painter
{
private:
    void BindTextureIfExist(std::string textureNameInShader, Texture* texture);
    
protected:

    enum CommonUniform { LightPosViewEnum, ModelViewEnum, normalToViewEnum, MVPEnum};

    void SkipCommonUniformToSendPerObject(CommonUniform s);

    std::vector<bool> mSkipUniforms;
    // Common painter functionalities:

    // * combine object's model matrix with stage's camera view, projection matrix
    // * send those matrix to shader
    // * bind object VAO
    // * bind object texture
    // This function ASSUMES that all Shaders will use uniform with those names:
    //  1. uniform vec4 LightPosView: the light position in View space
    //  2. uniform mat4 ModelView: the model view matrix = view matrix( from stage'scamera) * model matrix(from object)
    //  3. uniform mat3 normalToView: the normal transformation matrix, built from ModelView matrix in 2.
    //  4. uniform mat4 MVP: the modelViewProjection matrix, built from ModelView above * stage's projectionMatrix
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