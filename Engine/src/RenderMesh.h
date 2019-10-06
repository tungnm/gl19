#pragma once

#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <string>

/*
 * This class represent a mesh, loaded from OBJ file.
 * The class use Assimp to load the wavefront obj file and
 * upload the mesh data: position, normal, indices, etc..
 * to GPU memory. It keeps track of all buffer handles:
 * vbo handles, vao handles
 */
class Mesh
{

private:
    GLuint mVboHandles[3];
    GLuint mVaoHandle;
    glm::mat4 mModelMatrix;
    unsigned int mIndiceSize;

public:
    Mesh();
    ~Mesh();

    bool LoadObjFile(std::string fileName);

    void BindBuffers();

    unsigned int GetIndiceSize();

};