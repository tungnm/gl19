#include "DeferredPainter.h"


void GBufferPainter::Init()
{
    // shader load
    mShaderProgram.LoadShader("GbufferBuildVert", "buildGBuffer.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("GbufferBuildFrag", "buildGBuffer.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("GbufferBuildVert", "GbufferBuildFrag");

    // GBuffer Creation
    // creation of frame buffer for G-buffer with multiple color
    // buffers and a single depth buffer
    glGenFramebuffers(1, &mGBufferFrameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, mGBufferFrameBufferHandle);
    
    // create the texture to store position data and attach it to
    // the color attachment 0 of G-buffer(frame buffer)
    glGenTextures(1, &mGPositionTexHandle);
    glBindTexture(GL_TEXTURE_2D, mGPositionTexHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 768, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mGPositionTexHandle, 0);

    // - normal, attach to color attachment 1
    glGenTextures(1, &mGNormalTexHandle);
    glBindTexture(GL_TEXTURE_2D, mGNormalTexHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 768, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mGNormalTexHandle, 0);

    // - color, attach to color attachment 2
    glGenTextures(1, &mGColorTexHandle);
    glBindTexture(GL_TEXTURE_2D, mGColorTexHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mGColorTexHandle, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    // set the attachment to the currently bind frame buffer
    glDrawBuffers(3, attachments);
}

GBuffer GBufferPainter::GetGBuffer()
{
    GBuffer result = {mGPositionTexHandle, mGNormalTexHandle, mGColorTexHandle};
    return result;
}

void GBufferPainter::DrawObjects()
{
    // bind to G-buffer to draw on
    glBindFramebuffer(GL_FRAMEBUFFER, mGBufferFrameBufferHandle);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mShaderProgram.MakeCurrent();

    for (auto obj : mObjects)
    {
        //glm::mat4 MVP = CalculateMVP(obj, mStage);
        //mShaderProgram.SendMat4Uniform("MVP", MVP);
        CalculateAndSendObjectUniforms(&mShaderProgram, obj, mStage);
        // object color
        mShaderProgram.SendVec3Uniform("ObjectColor", obj->mMaterial.mColor);

        BindObjectVaoAndTexture(obj);
         
        // common render method
        RenderObject(obj);
    }
}

GBufferPainter::GBufferPainter()
{
    mGBufferFrameBufferHandle = 0;
}

GBufferPainter::~GBufferPainter() {}


DeferredPhongPainter::DeferredPhongPainter(GBuffer gBuffer, Mesh* quadMesh)
    : mGBuffer(gBuffer), mQuadMesh(quadMesh) {}

DeferredPhongPainter::~DeferredPhongPainter() {}

void DeferredPhongPainter::Init()
{
    // shader load
    mShaderProgram.LoadShader("deferPhongVert", "deferred.phong.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("deferePhongFrag", "deferred.phong.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("deferPhongVert", "deferePhongFrag");

}

void DeferredPhongPainter::DrawObjects()
{
    mShaderProgram.MakeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    // bind the textures of G-buffer, generated previously to the 
    // texture unit 0 , 1, 2 respectively, ready to be sampled by this Painter

    BindToTextureUnit(0, mGBuffer.mPositionTextureHandle);
    BindToTextureUnit(1, mGBuffer.mNormalTextureHandle);
    BindToTextureUnit(2, mGBuffer.mColorTextureHandle);
    
    // calculate & send LightPosView;
    // todo: move this to a common method in Painter if can reuse

    glm::mat4 view = glm::lookAt(
        mStage->GetCamera().mPosition, // camera pos in world space
        mStage->GetCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::vec4 lightPosView = view * glm::vec4(mStage->GetLight()[0].mPositon, 1.0f);

    mShaderProgram.SendVec4Uniform("LightPosView", lightPosView);

    mQuadMesh->BindBuffers();

    glDrawElements(
        GL_TRIANGLES,
        mQuadMesh->GetIndiceSize(),
        GL_UNSIGNED_INT,
        (void*)0);

}