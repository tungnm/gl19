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
#include "Object.h"
#include "Painter.h"
#include "DeferredPainter.h"

glm::vec3 camVec(0, 0, 0);
Stage stage1;
Physical dinoPhysical;
unsigned int dinoRotateDegree = 0;
int dinoRotateVelo = 1;

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
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        // stop/ start rotating the dino
        dinoRotateVelo = dinoRotateVelo == 0 ? 1 : 0;
    }
    else if (action == GLFW_RELEASE)
    {
        camVec = glm::vec3(0, 0, 0);
    }
}

void gameLogic()
{
    stage1.MoveCamera(camVec);
    dinoRotateDegree = (dinoRotateDegree + dinoRotateVelo) % 360;
    dinoPhysical.mOrientationDegree = (float)dinoRotateDegree;
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

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    
    Texture dinoDiffuse;
    dinoDiffuse.LoadSingleImage("stego.jpg");

    Texture dinoNormal;
    dinoNormal.LoadSingleImage("stego_normal.jpg");
    

    Mesh m1;
    //m1.LoadObjFile("stego.obj", true, true);
    m1.LoadObjFile("stego.obj", false , false);

    Mesh m2;
    m2.LoadObjFile("box.obj", false, false);

    Mesh quadMesh;
    quadMesh.LoadObjFile("quad.obj", true, false);

    dinoPhysical = {glm::vec3(0,0,0), 0, glm::vec3(0.15f ,0.15f ,0.15f )};
    //Object dino(&dinoPhysical, &m1, glm::vec3(0.9f, 0.5f, 0.3f), &dinoDiffuse, &dinoNormal);
    Object dino(&dinoPhysical, &m1, glm::vec3(0.9f, 0.5f, 0.3f));

    Physical boxPhysical{ glm::vec3(0.0, -50, 0.0), 0, glm::vec3(0.3f, 0.05f, 0.3f) };
    Object box(&boxPhysical, &m2, glm::vec3(0.3f, 0.5f, 0.9f));

    Physical box2Physical{ glm::vec3(80, -20, 80), 0, glm::vec3(0.02f, 0.08f, 0.02f) };
    Object box2(&box2Physical, &m2, glm::vec3(0.3f, 0.9f, 0.5f));

    /*
    PhongNormalMapPainter phong;
    phong.Init();
    //goraud.AssignObjects(&dino);
    phong.AssignObjects(&dino);
    */

    GBufferPainter gBuilderPainter;
    gBuilderPainter.Init();
    gBuilderPainter.AssignObjects(&box);
    gBuilderPainter.AssignObjects(&box2);
    gBuilderPainter.AssignObjects(&dino);


    SSAOPainter ssaoPainter(gBuilderPainter.GetGBuffer(), &quadMesh);
    ssaoPainter.Init();
    

    glm::vec3 camPos(2, 1, 5);
    glm::vec3 camLook(0, 0, 0);

    stage1.SetCamera(glm::vec3(2, 1, 5), glm::vec3(0, 0, 0));
    stage1.AddLight(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(0.8, 0.8, 0.8));
    stage1.SetProjectionMatrix(
        glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 3.0f, 20.0f)

    );
    /*
    goraud.AssignStage(&stage1);
    phong.AssignStage(&stage1);
    */

    std::vector<Object*> objects;
    objects.push_back(&dino);
    objects.push_back(&box);
    objects.push_back(&box2);

    ShadowMapPainter shadowPainter(objects, stage1.GetLight()[0]);
    shadowPainter.Init();

    DeferredPhongPainter gPhongPainter(
        gBuilderPainter.GetGBuffer(), &quadMesh, ssaoPainter.GetSSAOTextureHandle(),
        shadowPainter.GetShadowMapTextureHandle());
    gPhongPainter.Init();

    gBuilderPainter.AssignStage(&stage1);
    gPhongPainter.AssignStage(&stage1);
    ssaoPainter.AssignStage(&stage1);

    /*
    GouraudPainter goraud(shadowPainter.GetShadowMapTextureHandle()) ;
    goraud.Init();
    goraud.AssignObjects(&dino);
    goraud.AssignObjects(&box);
    goraud.AssignObjects(&box2);
    goraud.AssignStage(&stage1);
    */
    TexturePainter texturePainter(shadowPainter.GetShadowMapTextureHandle(), &quadMesh);
    texturePainter.Init();

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, key_callback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        gameLogic();

        /*
        glClear and framebuffer:
        By binding to the GL_FRAMEBUFFER target all the next read
        and write framebuffer operations will affect the currently
        bound framebuffer. It is also possible to bind a framebuffer
        to a read or write target specifically by binding to GL_READ_FRAMEBUFFER
        or GL_DRAW_FRAMEBUFFER respectively. The framebuffer bound
        to GL_READ_FRAMEBUFFER is then used for all read operations like 
        glReadPixels and the framebuffer bound to GL_DRAW_FRAMEBUFFER 
        is used as the destination for rendering, clearing and other
        write operations. Most of the times you won't need to make
        this distinction though and you generally bind to both with
        GL_FRAMEBUFFER.

        */
        
        gBuilderPainter.DrawObjects();

        ssaoPainter.DrawObjects();
        
        gPhongPainter.DrawObjects();

        //shadowPainter.DrawObjects();
        //goraud.DrawObjects();
        //texturePainter.DrawObjects();

        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}