#pragma once

#include "Object.h"
#include <vector>
#include "ShaderLoader.h"
class Painter
{
 
protected:

    void BindToTextureUnit(int shaderTextureUnit, Texture* texture);
    void BindToTextureUnit(int shaderTextureUnit, GLuint textureHandle);

    // Common painter functionalities:

    // * combine object's model matrix with stage's camera view, projection matrix
    // * send those matrix to shader
    // This function ASSUMES that all Shaders will use uniform with those names:
    //  1. uniform vec4 LightPosView: the light position in View space
    //  2. uniform mat4 ModelView: the model view matrix = view matrix( from stage'scamera) * model matrix(from object)
    //  3. uniform mat3 normalToView: the normal transformation matrix, built from ModelView matrix in 2.
    //  4. uniform mat4 MVP: the modelViewProjection matrix, built from ModelView above * stage's projectionMatrix
    void CalculateAndSendObjectUniforms(
        ShaderProgram* shader,
        Object* obj,
        Stage* stage);

    // * bind object VAO
    // * bind object texture
    // This should be called before glDraw* call
    void BindObjectVaoAndTexture(Object* obj);

    glm::mat4 CalculateMVP(Object* obj, Stage* stage);

    void RenderMesh(Mesh* mesh);
    
    std::vector<Object* > mObjects;
    Stage* mStage;
    ShaderProgram mShaderProgram;

public:
    // todo: get rid of these 2 functions for Parent PAinter
    // class. Each Painter should take in what he needs in the
    // constructor
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
    unsigned int depthMapTextureHandle;
    // shader to render depth map from light view
    ShaderProgram mDepthShader;

public:
    void DrawObjects();
    void Init();

    GouraudPainter(GLuint depthMapTextureHandle);
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

class ShadowMapPainter : public Painter
{
private:
    Light mLight;
    GLuint depthMapFBO;
    GLuint depthMapTextureHandle;
    ShaderProgram mDepthShader;
    //todo: remove this from main Painter class
    std::vector<Object*> mobjects;
public:
    void DrawObjects();
    void Init();
    GLuint GetShadowMapTextureHandle();

    ShadowMapPainter(std::vector<Object*> objects, Light light);
    ~ShadowMapPainter();
};