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

void Painter::PrepareObjectForRender(Object* obj)
{
    mShaderProgram.MakeCurrent();
    // stage prepare
    glm::mat4 view = glm::lookAt(
        mStage->GetActiveCamera().mPosition, // camera pos in world space
        mStage->GetActiveCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    //                                 field of view.       aspect ratio   near and far plane
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 3.0f, 60.0f);

    // only do first light for now
    //glm::vec4 lightPosView = view * glm::vec4(mStage->GetLight()[0].mPositon, 1.0f);
    glm::vec4 lightPosView = view * glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
    glm::mat4 model = glm::scale(glm::mat4(1.0f), obj->mPhysical->mScale);
    model = glm::translate(model, obj->mPhysical->mPosition);
    model = glm::rotate(model, glm::radians(obj->mPhysical->mOrientationDegree), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 mv = view * model;
    glm::mat4 mvp = proj * view * model;

    // send per object uniform to shaders

    SendVec4UniformToShader("LightPosView", lightPosView);
    SendMat4UniformToShader("ModelView", mv);
    SendMat3UniformToShader(
        "normalToView",
        glm::inverseTranspose(glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]))));
    SendMat4UniformToShader("MVP", mvp);
    
    // bind VAO
    obj->mMesh->BindBuffers();
    
    // bind texture(s)
    BindTextureIfExist("DiffuseTexture", obj->mMaterial.mDiffuse);
    BindTextureIfExist("NormalMapTexture", obj->mMaterial.mNormal);

 }

void Painter::RenderObject(Object* obj)
{
    glDrawElements(
        GL_TRIANGLES,
        obj->mMesh->GetIndiceSize(),
        GL_UNSIGNED_INT,
        (void*)0);
}

void Painter::SendVec3UniformToShader(std::string uniformName, glm::vec3 value)
{
    GLint uniformLocation = glGetUniformLocation(mShaderProgram.GetProgramHandle(),
        uniformName.c_str());

    if (uniformLocation < 0)
    {
        std::cout << "\nError: could not locate shader uniform: " << uniformName;
    }

    glUniform3fv(uniformLocation, 1, glm::value_ptr(value));
}

void Painter::SendVec4UniformToShader(std::string uniformName, glm::vec4 value)
{
    GLint uniformLocation = glGetUniformLocation(mShaderProgram.GetProgramHandle(),
        uniformName.c_str());
    if (uniformLocation < 0)
    {
        std::cout << "\nError: could not locate shader uniform: " << uniformName;
    }
    glUniform4fv(uniformLocation, 1, glm::value_ptr(value));
}

void Painter::SendMat4UniformToShader(std::string uniformName, glm::mat4 value)
{
    GLint uniformLocation = glGetUniformLocation(mShaderProgram.GetProgramHandle(),
        uniformName.c_str());
    if (uniformLocation < 0)
    {
        std::cout << "\nError: could not locate shader uniform: " << uniformName;
    }
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
}

void Painter::SendMat3UniformToShader(std::string uniformName, glm::mat3 value)
{
    GLint uniformLocation = glGetUniformLocation(mShaderProgram.GetProgramHandle(),
        uniformName.c_str());
    if (uniformLocation < 0)
    {
        std::cout << "\nError: could not locate shader uniform: " << uniformName;
    }
    glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
}

Painter::Painter(Stage* stage) 
    : mStage(stage) {}

Painter::~Painter() {}

void GouraudPainter::DrawObjects()
{
    for (auto obj : mObjects)
    {
        // setup matrics, uniform, bind VAO.., common for all painters
        PrepareObjectForRender(obj);

        // painter specific logic: Gouraud need only color per object
        SendVec3UniformToShader("ObjectColor", obj->mMaterial.mColor);
        
        // common render method
        RenderObject(obj);
    }
}

void GouraudPainter::Init()
{
    mShaderProgram.LoadShader("basicVert", "gouraud.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("basicFrag", "gouraud.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("basicVert", "basicFrag");
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
    for (auto obj : mObjects)
    {
        // setup matrics, uniform, bind VAO.., common for all painters
        PrepareObjectForRender(obj);

        // painter specific logic: Gouraud need only color per object
       
        // common render method
        RenderObject(obj);
    }
}

PhongNormalMapPainter::PhongNormalMapPainter() : Painter() {}

PhongNormalMapPainter::~PhongNormalMapPainter() {}

