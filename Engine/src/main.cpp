
#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_inverse.hpp>
#include <fstream>
#include <sstream>
#include <memory>
#include "ShaderLoader.h"
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

void printMatrix(glm::mat4 m)
{
    for (int i = 0; i < 4; i++)
    {
        std::cout << std::endl;
        for (int j = 0; j < 4; j++)
        {
            std::cout << m[i][j] << "\t";
        }
    }
}

int main() {

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);


    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // need to call glad to load openGL function after context is created.
    if (!gladLoadGL()) {
        std::cerr << "Glad is unable to load openGL functions!\n";
        exit(EXIT_FAILURE);
    }

    ShaderManager shaman;
    shaman.LoadShader("basicVert","basic.vert.glsl", GL_VERTEX_SHADER);
    shaman.LoadShader("basicFrag","basic.frag.glsl", GL_FRAGMENT_SHADER);
    shaman.CreateProgram("default", "basicVert", "basicFrag");

    //load model


    std::unique_ptr<const struct aiScene> scene(aiImportFile("models\\stego.obj", aiProcessPreset_TargetRealtime_Quality));
    const struct aiMesh* mesh = scene->mMeshes[0];

    GLuint vboHandles[3];
    glGenBuffers(3, vboHandles);

    //load position
    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(float), mesh->mVertices, GL_STATIC_DRAW);

    //load normal
    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[1]);
    glBufferData(GL_ARRAY_BUFFER, mesh->mNumVertices * 3 * sizeof(float), mesh->mNormals, GL_STATIC_DRAW);

    GLuint vaoHandle;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
 
    std::vector<unsigned int> indices(mesh->mNumFaces * 3);

    unsigned int* current = & indices[0];

    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        memcpy(current, mesh->mFaces[i].mIndices, 3 * sizeof(unsigned int));
        current += 3;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandles[2]);//note that for indice, use GL_ELEMENT_ARRAY_BUFFER instead of GL_ARRAY_BUFFER
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, &indices[0], GL_STATIC_DRAW);
    glEnable(GL_DEPTH_TEST);
    float x = 0.0f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        //glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), float(angle), glm::vec3(0.0f, 0.0f, 1.0f));

        //angle += 1;
        //angle = angle % 360;

        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f ,0.1f ,0.1f ));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 view = glm::lookAt(
            glm::vec3(2, 0, 3 ), // Camera is at (4,3,3), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );
        x += 0.01;
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 80.0f);

        glm::mat4 mv = view * model;
        glm::mat4 mvp = proj * view * model;

        

        glm::vec4 lightPosView = mv * glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
        
  
        GLuint LightPosView = glGetUniformLocation(shaman.GetProgramHandle("default"),
            "LightPosView");
        GLuint ModelView = glGetUniformLocation(shaman.GetProgramHandle("default"),
            "ModelView");
        GLuint normalToView = glGetUniformLocation(shaman.GetProgramHandle("default"),
            "normalToView");
        GLuint MVP = glGetUniformLocation(shaman.GetProgramHandle("default"),
            "MVP");

        if (LightPosView >= 0)
        {
            glUniform4fv(LightPosView, 1, glm::value_ptr(lightPosView));
            //tofo: separate if, lol
            glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(mv));
            glUniformMatrix3fv(normalToView, 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])))));
            glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(mvp));

        }

        glBindVertexArray(vaoHandle);
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}