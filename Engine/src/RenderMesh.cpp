#pragma once

#include "RenderMesh.h"
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <memory>
#include <fstream>
#include "Util.h"
#include <vector>
#include <iostream>
#include <glm.hpp>

#include <gtc/matrix_transform.hpp>

void delete_assimp_scene_func(const struct aiScene* scene)
{
    aiReleaseImport(scene);
}

bool Mesh::LoadObjFile(std::string fileName)
{
    std::string modelPath = "models\\" + fileName;

    if (!isFileExist(modelPath))
    {
        return false;
    }
    
    const struct aiScene* scene = aiImportFile(modelPath.c_str(), aiProcessPreset_TargetRealtime_Quality);

    if (scene == nullptr)
    {
        std::cout << "\nAssimp failed to load the file " << modelPath;
        return false;
    }

    // this is C++ 11 custom deleter for unique_ptr in action ^^
    std::unique_ptr< const struct aiScene, void(*)(const struct aiScene*)> scenePtr(scene, delete_assimp_scene_func);

    const struct aiMesh* mesh = scene->mMeshes[0];
    
    /* assimps store indices in a struct of type: {int numIndice, int first, int second, int third}
       This is to support mesh with face of mixed type: ie: quad mixes with triangles. However, we 
       only use triangle only mesh and just need the first, second, third part of the struct.
       So need to copy all indices into a 1D array to copy into VBO(and drop the numIndice part)
     */
    std::vector<unsigned int> indices(mesh->mNumFaces * 3);

    mIndiceSize = mesh->mNumFaces * 3;

    unsigned int* current = &indices[0];

    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        memcpy(current, mesh->mFaces[i].mIndices, 3 * sizeof(unsigned int));
        current += 3;
    }
    // in the order: position, normal, indices, texture cord
    glGenBuffers(5, mVboHandles);

    //load position
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(float), mesh->mVertices, GL_STATIC_DRAW);

    //load normal
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[1]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(float), mesh->mNormals, GL_STATIC_DRAW);

    //load face indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboHandles[2]);//note that for indice, use GL_ELEMENT_ARRAY_BUFFER instead of GL_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, &indices[0], GL_STATIC_DRAW);

    //load texture coordinates
    //NOTE: loading image with stb, need to revert the y coordinate!!!
    std::vector<float> texCoords(sizeof(float) * 2 * mesh->mNumVertices);
    for (unsigned int k = 0; k < mesh->mNumVertices; ++k)
    {

        texCoords[k * 2] = mesh->mTextureCoords[0][k].x;
        texCoords[k * 2 + 1] = -mesh->mTextureCoords[0][k].y;

    }
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[3]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 2 * sizeof(float), &texCoords[0], GL_STATIC_DRAW);

    //load tangent
    // calculate handedness for tangent array
    GLfloat* tang = (GLfloat*)malloc(sizeof(GLfloat) * 4 * mesh->mNumVertices);
    for (int i = 0; i < mesh->mNumVertices; ++i)
    {
        glm::vec3 t = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        glm::vec3 b = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
        glm::vec3 n = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        
        glm::mat3 tbn(1.0f); // identity matric
        tbn[0] = t; //first column
        tbn[1] = b; //second column
        tbn[2] = n; //third column

        tang[i * 4] = t.x;
        tang[i * 4 + 1] = t.y;
        tang[i * 4 + 2] = t.z;
        //determine handedness:
        float handedness = glm::determinant(tbn);
        //	GLfloat w=( a< 0.0f) ? -1.0f : 1.0f;
        tang[i * 4 + 3] = handedness < 0 ? -1.0f : 1.0f;

    }
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[4]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 4 * sizeof(float), tang, GL_STATIC_DRAW);

    delete[] tang;

    // VAO is the mapping of which buffer go to which input variable in the vertex shader and how
    glGenVertexArrays(1, &mVaoHandle);
    glBindVertexArray(mVaoHandle);

    // Enable layout=0 input variable in vertex shader
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    // assign position buffer to first input variable of vertex shader(layout = 0)
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // assign normal buffer to second input variable of vertex shader(layout = 1)
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // assign text coord
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[3]);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    // assign tangent
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[4]);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    std::cout << "\nSuccessfully loaded: " << modelPath;
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{
    for (int i = 0; i < 3; i++)
    {
        if (mVboHandles[i] > 0)
        {
            std::cout << "\nDeleting VBO handle: " << mVboHandles[i];
            glDeleteBuffers(1, &mVboHandles[i]);
        }
    }

    if (mVaoHandle > 0)
    {
        std::cout << "\nDeleting VAO  handle: " << mVaoHandle;
        glDeleteVertexArrays(1, &mVaoHandle);
    }
    

}

void Mesh::BindBuffers()
{
    glBindVertexArray(mVaoHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboHandles[2]);
}

unsigned int Mesh::GetIndiceSize()
{
    return mIndiceSize;
}