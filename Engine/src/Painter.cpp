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

void Painter::BindTextureIfExist(std::string textureNameInShader, Texture* texture)
{
    // todo: right now send texture to shader if texture exists in the Object
    // should do the otherway around: need to associate which texture a shader
    // needs, and then look for that in the object
    // check if the object Material contains any texture. Bind if any exist
    if (texture != nullptr)
    {
        GLint textureLocationInShader = glGetUniformLocation(
            mShaderProgram.GetProgramHandle(),
            textureNameInShader.c_str());
        if (textureLocationInShader < 0) {
            std::cout << "\nError: texture unit name " << textureNameInShader << " not found in shader";
            return;
        }
        else
        {
            glUniform1i(textureLocationInShader, texture->GetTextureUnit());
        }
    }
}

void Painter::PrepareObjectForRender(
    ShaderProgram* shader,
    Object* obj,
    Stage* stage)
{
    // stage prepare: grab view matrix and projection matrix from mStage
    glm::mat4 view = glm::lookAt(
        stage->GetActiveCamera().mPosition, // camera pos in world space
        stage->GetActiveCamera().mTarget, // camera target
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
    
    // bind VAO
    obj->mMesh->BindBuffers();
    
    // todo: fix this texture binding per object
    // bind texture(s)
    //BindTextureIfExist("DiffuseTexture", obj->mMaterial.mDiffuse);
    //BindTextureIfExist("NormalMapTexture", obj->mMaterial.mNormal);

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
    : mStage(stage) {}

Painter::~Painter() {}

glm::mat4 Painter::CalculateMVP(Object* obj, Stage* stage)
{
    glm::mat4 view = glm::lookAt(
        stage->GetActiveCamera().mPosition, // camera pos in world space
        stage->GetActiveCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    glm::mat4 model = glm::scale(glm::mat4(1.0f), obj->mPhysical->mScale);
    model = glm::translate(model, obj->mPhysical->mPosition);
    model = glm::rotate(model, glm::radians(obj->mPhysical->mOrientationDegree), glm::vec3(0.0f, 1.0f, 0.0f));

    return stage->GetProjectionMatrix() * view * model;
}

void GouraudPainter::DrawObjects()
{
    mDepthShader.MakeCurrent();
    
    // first pass shadow
    // 1. first render to depth map
    glViewport(0, 0, 1024, 768);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // set the camera view from the light source, looking into the scene
    // also set the projection matrix for the light view to be very tight,
    // to increase precision for z buffer.
    Stage lightStage;
    lightStage.AddCamera("test", mStage->GetLight()[0].mPositon, glm::vec3(0,0,0));
    lightStage.AddLight(glm::vec3(10, 10, 10));
    lightStage.SetProjectionMatrix(
                                                              // tight frustum
        glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 14.0f, 22.0f)
    );

    for (auto obj : mObjects)
    {
        // setup matrics, uniform, bind VAO.., common for all painters
        PrepareObjectForRender(&mDepthShader, obj, &lightStage);
         
        // common render method
        RenderObject(obj);
    }
    
    // second pass
    //mStage->SetActiveCamera("mainCam");
    mShaderProgram.MakeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 1024, 768);
    //glBindTexture()
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    for (auto obj : mObjects)
    {
        // calculate lightMVP to send to shader
        glm::mat4 lightMVP = CalculateMVP(obj, &lightStage);
        mShaderProgram.SendMat4Uniform("LightMVP", lightMVP);
       
        // setup matrics, uniform, bind VAO.., common for all painters
        PrepareObjectForRender(&mShaderProgram, obj, mStage);
        // painter specific logic: Gouraud need only color per object
        mShaderProgram.SendVec3Uniform("ObjectColor", obj->mMaterial.mColor);

        
        glBindTexture(GL_TEXTURE_2D, depthMap);

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
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 768;

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //With the generated depth texture we can attach it as the framebuffer's depth buffer:
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    
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
        PrepareObjectForRender(&mShaderProgram, obj, mStage);
       
        // common render method
        RenderObject(obj);
    }
}

PhongNormalMapPainter::PhongNormalMapPainter() : Painter() {}

PhongNormalMapPainter::~PhongNormalMapPainter() {}

