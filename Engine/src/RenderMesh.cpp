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

    // in the order: position, normal, indices
    glGenBuffers(3, mVboHandles);

    //load position
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(float), mesh->mVertices, GL_STATIC_DRAW);

    //load normal
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[1]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(float), mesh->mNormals, GL_STATIC_DRAW);

    //load face indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboHandles[2]);//note that for indice, use GL_ELEMENT_ARRAY_BUFFER instead of GL_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, &indices[0], GL_STATIC_DRAW);

    // VAO is the mapping of which buffer go to which input variable in the vertex shader and how
    glGenVertexArrays(1, &mVaoHandle);
    glBindVertexArray(mVaoHandle);

    // Enable layout=0 input variable in vertex shader
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // assign position buffer to first input variable of vertex shader(layout = 0)
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // assign normal buffer to first input variable of vertex shader(layout = 0)
    glBindBuffer(GL_ARRAY_BUFFER, mVboHandles[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    std::cout << "Successfully loaded: " << modelPath;
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{
    for (int i = 0; i < 3; i++)
    {
        std::cout << "\nDeleting VBO handle: " << mVboHandles[i];
        glDeleteBuffers(1, &mVboHandles[i]);
    }

    std::cout << "\nDeleting VAO  handle: " << mVaoHandle;

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