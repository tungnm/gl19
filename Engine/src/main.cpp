//Windows only support openGL 1.1 header. To have newer version of openGL
//need to include glad: https://glad.dav1d.de/
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
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include "ShaderLoader.h"
#include "RenderMesh.h"

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
    shaman.LoadShader("basicVert","cellShade.vert.glsl", GL_VERTEX_SHADER);
    shaman.LoadShader("basicFrag","cellShade.frag.glsl", GL_FRAGMENT_SHADER);
    shaman.CreateProgram("default", "basicVert", "basicFrag");
    
    Mesh m1;
    m1.LoadObjFile("stego.obj");

    int x = 0;

    glEnable(GL_DEPTH_TEST);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f ,0.1f ,0.1f ));
        model = glm::rotate(model, glm::radians(float(x)), glm::vec3(0.0f, 1.0f, 0.0f));
        x = (x + 1) % 360;
        glm::mat4 view = glm::lookAt(
            glm::vec3(2, 1, 5 ), // Camera is at (2, 0, 3), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        //                                 field of view.       aspect ratio   near and far plane
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 80.0f);

        glm::mat4 mv = view * model;
        glm::mat4 mvp = proj * view * model;

        glm::vec4 lightPosView = view * glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
        
  
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

        m1.BindBuffers();

        glDrawElements(GL_TRIANGLES, m1.GetIndiceSize(), GL_UNSIGNED_INT, (void*)0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}