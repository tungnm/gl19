#include "Object.h"

Object::Object()
{
    mMesh = nullptr;
    mMaterial.mColor = glm::vec3(0.9f, 0.5f, 0.3f); // default color
    mMaterial.mDiffuse = nullptr;
    mMaterial.mNormal = nullptr;
    mMaterial.mHeight = nullptr;

}

Object::Object(
    Physical* physical,
    Mesh* mesh,
    glm::vec3 color,
    Texture* diffuseMap,
    Texture* normalMap,
    Texture* heightMap)
    : mPhysical(physical), mMesh(mesh)
{
    mMaterial.mColor = color;
    mMaterial.mDiffuse = diffuseMap;
    mMaterial.mNormal = normalMap;
    mMaterial.mHeight = heightMap;
}


// donothing on destructor. Object is just a lightweight
// class and does not handle freeing GPU resource, etc..
Object::~Object(){}

Stage::Stage()
{
    // default projection matrix
    mProjectionMatrix = glm::perspective(
        glm::radians(45.0f),
        4.0f / 3.0f,
        3.0f,
        60.0f);

    // default camera
    mCamera.mPosition = glm::vec3(6,6,6);
    mCamera.mTarget = glm::vec3(0,0,0);
}

Stage::~Stage() {}

void Stage::SetProjectionMatrix(glm::mat4 projMat)
{
    mProjectionMatrix = projMat;
}

glm::mat4 Stage::GetProjectionMatrix()
{
    return mProjectionMatrix;
}

void Stage::SetCamera(
    glm::vec3 cameraPosition,
    glm::vec3 cameraTarget)
{

    Camera newCamera;
    newCamera.mPosition = cameraPosition;
    newCamera.mTarget = cameraTarget;

    mCamera.mPosition = cameraPosition;
    mCamera.mTarget = cameraTarget;
}

void Stage::MoveCamera(glm::vec3 delta)
{
    mCamera.mPosition += delta;
}

Camera Stage::GetCamera()
{
    return mCamera;
}

void Stage::AddLight(glm::vec3 position, glm::vec3 color)
{
    Light newLight;
    newLight.mPositon = position;
    newLight.mColor = color;

    mLights.push_back(newLight);
}

std::vector<Light> Stage::GetLight()
{
    return mLights;
}