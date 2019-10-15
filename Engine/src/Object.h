#pragma once

#include "RenderMesh.h"
#include <glm.hpp>
#include "Texture.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

struct ObjectMaterial
{
    glm::vec3 mColor;
    Texture* mDiffuse;
    Texture* mNormal;
    Texture* mHeight;
};

struct Physical
{
    glm::vec3 mPosition;
    float mOrientationDegree;
    glm::vec3 mScale;
};

/*
    This lightweight class represent an object in 3D space:
        its body with a mesh
        its color and texture details
*/
class Object
{
public:
    Physical* mPhysical;
    Mesh* mMesh;
    ObjectMaterial mMaterial;

    Object(Physical* physical,
           Mesh* mesh,
           glm::vec3 color,
           Texture *diffuseMap = nullptr,
           Texture* normalMap = nullptr,
           Texture* heightMap = nullptr);

    Object();
    ~Object();
};

struct Camera
{
    glm::vec3 mPosition;
    glm::vec3 mTarget;
};

struct Light
{
    glm::vec3 mPositon;
    glm::vec3 mColor;
};

/*
    This class represent a stage:
    A list of cameras where one is active
    a list of lights that light up object in the stage
*/
class Stage
{
private:
    static const Camera DEFAULT_CAMERA;
    // keeps track of a list of cameras via names
    std::unordered_map<std::string, Camera> mCameras;
    glm::mat4 mProjectionMatrix;
    std::vector<Light> mLights;
    std::string mActiveCameraName;
public:
    Stage();
    ~Stage();

    void AddCamera(
        std::string cameraName,
        glm::vec3 cameraPosition,
        glm::vec3 cameraTarget);

    bool SetActiveCamera(std::string cameraName);

    // optional, doesn't need to call this one
    void SetProjectionMatrix(glm::mat4 projMat);

    glm::mat4 GetProjectionMatrix();

    void MoveCamera(std::string cameraName, glm::vec3 delta);

    Camera GetActiveCamera();

    void AddLight(
        glm::vec3 position,
        glm::vec3 color = glm::vec3(0.8, 0.8, 0.8));

    std::vector<Light> GetLight();

};