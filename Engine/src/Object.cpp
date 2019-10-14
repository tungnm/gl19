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

Stage::Stage() {}

Stage::~Stage() {}

void Stage::AddCamera(
    std::string cameraName,
    glm::vec3 cameraPosition,
    glm::vec3 cameraTarget)
{
    if (mCameras.empty())
    {
        mActiveCameraName = cameraName;
    }
    Camera newCamera;
    newCamera.mPosition = cameraPosition;
    newCamera.mTarget = cameraTarget;

    mCameras[cameraName] = newCamera;
}

bool Stage::SetActiveCamera(std::string cameraName)
{
    if (mCameras.count(cameraName) == 0)
    {
        std::cout << "\nError: Camera " << cameraName << " does not exist.";
        return false;
    }
    mActiveCameraName = cameraName;
}

void Stage::MoveCamera(std::string cameraName, glm::vec3 delta)
{
    mCameras[cameraName].mPosition += delta;
}

Camera Stage::GetActiveCamera()
{
    return mCameras[mActiveCameraName];
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