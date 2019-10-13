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
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "ShaderLoader.h"
#include "RenderMesh.h"
#include "Texture.h"

glm::vec3 camPos(2, 1, 5);
glm::vec3 camLook(0, 0, 0);

glm::vec3 camVec(0, 0, 0);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        camVec = glm::vec3(-0.05, 0, 0);
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        camVec = glm::vec3(0.05, 0, 0);
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        camVec = glm::vec3(0, 0.05, 0);
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        camVec = glm::vec3(0, -0.05, 0);
    }
    else if (action == GLFW_RELEASE)
    {
        camVec = glm::vec3(0, 0, 0);
    }
}





int main() {

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 768, "Hello World", NULL, NULL);


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
    shaman.LoadShader("basicVert","gouraud.vert.glsl", GL_VERTEX_SHADER);
    shaman.LoadShader("basicFrag","gouraud.frag.glsl", GL_FRAGMENT_SHADER);
    shaman.CreateProgram("default", "basicVert", "basicFrag");
    
    Mesh m1;
    m1.LoadObjFile("stego.obj");

    Mesh m2;
    m2.LoadObjFile("box.obj");

    Texture diffuse;
    diffuse.LoadSingleImage("bricks.jpg");

    Texture normal;
    normal.LoadSingleImage("bricksNormal.jpg");

    Texture height;
    height.LoadSingleImage("bricksDepth.jpg");
    
    GLuint colorTex = glGetUniformLocation(shaman.GetProgramHandle("default"), "Tex1");
    GLuint normapMapTex = glGetUniformLocation(shaman.GetProgramHandle("default"), "normalMapTex");
    GLuint heightMapTex = glGetUniformLocation(shaman.GetProgramHandle("default"), "heightMapTex");

    // Then bind the uniform samplers to texture units:
    glUseProgram(shaman.GetProgramHandle("default"));
    glUniform1i(colorTex, diffuse.GetTextureUnit());
    glUniform1i(normapMapTex, normal.GetTextureUnit());
    glUniform1i(heightMapTex, height.GetTextureUnit());

    int x = 0;

    glEnable(GL_DEPTH_TEST);


    glfwSetKeyCallback(window, key_callback);


    float li = 0;// 0.5;
    float vec = 0;// 0.1;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        camPos += camVec;
        //camLook += camVec;
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       
        glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f ,0.15f ,0.15f ));

        glm::mat4 boxModel = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.05f, 0.3f));
        boxModel = glm::translate(boxModel, glm::vec3(0.0, -50, 0.0));
        model = glm::rotate(model, glm::radians(float(x)), glm::vec3(0.0f, 1.0f, 0.0f));
        x = (x + 1) % 360;
        glm::mat4 view = glm::lookAt(
            camPos, // Camera is at (2, 0, 3), in World Space
            camLook, // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        //                                 field of view.       aspect ratio   near and far plane
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 80.0f);

        glm::mat4 mv = view * model;
        glm::mat4 mvp = proj * view * model;

        glm::vec4 lightPosView = view * glm::vec4(10.0f, 10.0f, 10.0f + li, 1.0f);
        li += vec;

        if (li > 20 ||  li < -20) {
            vec *= -1;
        }
  
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

        m2.BindBuffers();

        glm::mat4 proj2 = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 80.0f);

        glm::mat4 mv2 = view * boxModel;
        glm::mat4 mvp2 = proj * view * boxModel;
        glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(mv2));
        glUniformMatrix3fv(normalToView, 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(glm::vec3(mv2[0]), glm::vec3(mv2[1]), glm::vec3(mv2[2])))));
        glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(mvp2));

        glDrawElements(GL_TRIANGLES, m2.GetIndiceSize(), GL_UNSIGNED_INT, (void*)0);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}