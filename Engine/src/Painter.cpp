#include "Painter.h"
#include <gtc/matrix_inverse.hpp>
#include <iostream>

void Painter::AssignObjects(Object* object)
{
    mObjects.push_back(object);
}

void Painter::AssignStage(Stage* stage)
{
    mStage = stage;
}


void Painter::BindTextureIfExist(int shaderTextureUnit, Texture* texture)
{
    if (texture != nullptr)
    {
        glActiveTexture(GL_TEXTURE0 + shaderTextureUnit);
        glBindTexture(GL_TEXTURE_2D, texture->GetTextureHandle());
    }
}

void Painter::BindTextureIfExist(int shaderTextureUnit, GLuint textureHandle)
{
    if (textureHandle > 0)
    {
        glActiveTexture(GL_TEXTURE0 + shaderTextureUnit);
        glBindTexture(GL_TEXTURE_2D, textureHandle);
    }
}

void Painter::CalculateAndSendObjectUniforms(
    ShaderProgram* shader,
    Object* obj,
    Stage* stage)
{
    // stage prepare: grab view matrix and projection matrix from mStage
    glm::mat4 view = glm::lookAt(
        stage->GetCamera().mPosition, // camera pos in world space
        stage->GetCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // only do first light for now
    glm::vec4 lightPosView = view * glm::vec4(stage->GetLight()[0].mPositon, 1.0f);

    glm::mat4 model = glm::scale(glm::mat4(1.0f), obj->mPhysical->mScale);
    model = glm::translate(model, obj->mPhysical->mPosition);
    model = glm::rotate(model, glm::radians(obj->mPhysical->mOrientationDegree), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 mv = view * model;
    glm::mat4 mvp = stage->GetProjectionMatrix() * view * model;

    // send per object uniform to shaders

    shader->SendVec4Uniform("LightPosView", lightPosView);
    shader->SendMat4Uniform("ModelView", mv);
    shader->SendMat3Uniform(
        "normalToView",
        glm::inverseTranspose(glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]))));
    shader->SendMat4Uniform("MVP", mvp);
    
}

void Painter::BindObjectVaoAndTexture(Object* obj)
{
    // bind VAO
    obj->mMesh->BindBuffers();

    // bind texture(s)
    // always assume that, in shader code texture unit 0 is for diffuse sampler
    // texture unit 1 is for normal sampler
    BindTextureIfExist(0, obj->mMaterial.mDiffuse);
    BindTextureIfExist(1, obj->mMaterial.mNormal);
}

void Painter::RenderObject(Object* obj)
{
    glDrawElements(
        GL_TRIANGLES,
        obj->mMesh->GetIndiceSize(),
        GL_UNSIGNED_INT,
        (void*)0);
}

Painter::Painter(Stage* stage) 
    : mStage(stage) 
{
       
}

Painter::~Painter() {}

glm::mat4 Painter::CalculateMVP(Object* obj, Stage* stage)
{
    glm::mat4 view = glm::lookAt(
        stage->GetCamera().mPosition, // camera pos in world space
        stage->GetCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    glm::mat4 model = glm::scale(glm::mat4(1.0f), obj->mPhysical->mScale);
    model = glm::translate(model, obj->mPhysical->mPosition);
    model = glm::rotate(model, glm::radians(obj->mPhysical->mOrientationDegree), glm::vec3(0.0f, 1.0f, 0.0f));

    return stage->GetProjectionMatrix() * view * model;
}

void GouraudPainter::DrawObjects()
{
    if (mStage == nullptr)
    {
        std::cout << "\nDrawObjects: mStage is null";
        return;
    }
    mDepthShader.MakeCurrent();
    
    // todo: maybe have this pass as a separate painter, to reuse
    // the shadow map for other shading, ie: Phong with shadow map?
    // first pass shadow
    // 1. first render to depth map
    glViewport(0, 0, 1280, 960);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // set the camera view from the light source, looking into the scene
    // also set the projection matrix for the light view to be very tight,
    // to increase precision for z buffer.
    Stage lightStage;
    lightStage.SetCamera(mStage->GetLight()[0].mPositon, glm::vec3(0,0,0));
    lightStage.AddLight(glm::vec3(10, 10, 10));
    lightStage.SetProjectionMatrix(
                                                              // tight frustum
        glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 14.0f, 22.0f)
    );

    for (auto obj : mObjects)
    {
        // shadow vert shader only need MVP to transform to light space, no need
        // to use the common CalculateAndSendObjectUniforms, which send
        // M, MV, MVP.
        glm::mat4 lightMVP = CalculateMVP(obj, &lightStage);
        mDepthShader.SendMat4Uniform("MVP", lightMVP);
        BindObjectVaoAndTexture(obj);
         
        // common render method
        RenderObject(obj);
    }
    
    // second pass
    mShaderProgram.MakeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1024, 768);
    //glBindTexture()
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // assume that depth map sampler is using texture unit 2 in shader(layout (binding=2))
    BindTextureIfExist(2, depthMapTextureHandle);

    for (auto obj : mObjects)
    {
        // calculate lightMVP to send to shader
        glm::mat4 lightMVP = CalculateMVP(obj, &lightStage);
        mShaderProgram.SendMat4Uniform("LightMVP", lightMVP);
       
        // setup matrics, uniform, bind VAO.., common for all painters
        CalculateAndSendObjectUniforms(&mShaderProgram, obj, mStage);
        BindObjectVaoAndTexture(obj);
        // painter specific logic: Gouraud need only color per object
        mShaderProgram.SendVec3Uniform("ObjectColor", obj->mMaterial.mColor);

        // common render method
        RenderObject(obj);
    }
    
}

void GouraudPainter::Init()
{
    // main rendering shader
    mShaderProgram.LoadShader("basicVert", "gouraud.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("basicFrag", "gouraud.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("basicVert", "basicFrag");

    // depth shader
    mDepthShader.LoadShader("depthVert", "depth.vert.glsl", GL_VERTEX_SHADER);
    mDepthShader.LoadShader("depthFrag", "depth.frag.glsl", GL_FRAGMENT_SHADER);
    mDepthShader.CreateProgram("depthVert", "depthFrag");

    //shadow map:
    //First we'll create a framebuffer object for rendering the depth map:

    glGenFramebuffers(1, &depthMapFBO);

    // Next we create a 2D texture that we'll use as the framebuffer's depth buffer:
    const unsigned int SHADOW_WIDTH = 1280, SHADOW_HEIGHT = 960;

    glGenTextures(1, &depthMapTextureHandle);
    glBindTexture(GL_TEXTURE_2D, depthMapTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //With the generated depth texture we can attach it as the framebuffer's depth buffer:
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTextureHandle, 0);
    
    /*
    A framebuffer object however is not complete without a color buffer so
    we need to explicitly tell OpenGL we're not going to render any color data.
    We do this by setting both the read and draw buffer to GL_NONE with
    glDrawBuffer and glReadbuffer.
    */

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

GouraudPainter::GouraudPainter() : Painter() {}

GouraudPainter::~GouraudPainter() {}

void PhongNormalMapPainter::Init()
{
    mShaderProgram.LoadShader("basicVert", "phong.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("basicFrag", "phong.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("basicVert", "basicFrag");
}

void PhongNormalMapPainter::DrawObjects()
{
    mShaderProgram.MakeCurrent();

    for (auto obj : mObjects)
    {
        // setup matrics, uniform, bind VAO.., common for all painters
        CalculateAndSendObjectUniforms(&mShaderProgram, obj, mStage);
        BindObjectVaoAndTexture(obj);
        // common render method
        RenderObject(obj);
    }
}

PhongNormalMapPainter::PhongNormalMapPainter() : Painter() {}

PhongNormalMapPainter::~PhongNormalMapPainter() {}

