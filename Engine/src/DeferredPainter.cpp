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

    // need to create a depth buffer for this custom made framebuffer so that depth test will work
    // depth buffer is a special RenderBuffer type, and not a Texture2D as the other color buffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    // - tell OpenGL the mapping from fragment shader out variables to G-buffer textures(color attachments)
    // For example. this means: set  the output "layout (location = 0) out vec3 GBufferPos;" in fragment shader to be render to
    // GL_COLOR_ATTACHMENT0 of frame buffer in this case(which is actually a texture, mGPositionTexHandle in this case)
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    // set the attachment to the currently bind frame buffer
    // note: this is different from glDrawBuffer (without s at the end)
    glDrawBuffers(3, attachments);

    // good practice to bind back to default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        RenderMesh(obj->mMesh);
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
    mShaderProgram.LoadShader("quadVert", "fullScreenQuad.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("deferePhongFrag", "deferred.phong.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("quadVert", "deferePhongFrag");

}

void DeferredPhongPainter::DrawObjects()
{
    mShaderProgram.MakeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    RenderMesh(mQuadMesh);

}

void TexturePainter::DrawObjects()
{
    mShaderProgram.MakeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    BindToTextureUnit(0, mTextureHandle);

    mQuadMesh->BindBuffers();

    RenderMesh(mQuadMesh);
}

void TexturePainter::Init()
{
    // load shader
    mShaderProgram.LoadShader("quadVert", "fullScreenQuad.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("textureRenderFrag", "renderTexture.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("quadVert", "textureRenderFrag");

}

TexturePainter::TexturePainter(GLuint textureHandle, Mesh* quadMesh)
: mTextureHandle(textureHandle), mQuadMesh(quadMesh) {}

TexturePainter::~TexturePainter() {}
