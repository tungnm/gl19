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
 * vbo handles, vao handles.

 * This class assump the shader use the VAO with layout as follow:
 * - position is at layout = 0
 * - normal is at layout = 1
 * - textcord is at layout = 2 (always)
 * - tangent is at layout = 3 (always, even if texcord is missing)
 */
class Mesh
{

private:
    GLuint mVboHandles[5];
    GLuint mVaoHandle;
    glm::mat4 mModelMatrix;
    unsigned int mIndiceSize;

public:
    Mesh();
    ~Mesh();

    bool LoadObjFile(std::string fileName, bool needTexCoord, bool needTangent);
    void BindBuffers();
    unsigned int GetIndiceSize();

};