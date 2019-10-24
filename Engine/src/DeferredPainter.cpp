#include "DeferredPainter.h"
#include <gtc/type_ptr.hpp>



class Random {
private:
    std::mt19937 generator;
    std::uniform_real_distribution<float> distr01;

public:
    Random() : distr01(0.0f, 1.0f) {
        std::random_device rd;
        generator.seed(rd());
    }

    float nextFloat() {
        return distr01(generator);
    }

    static void shuffle(std::vector<GLfloat>& v) {
        auto rng = std::default_random_engine{};
        std::shuffle(v.begin(), v.end(), rng);
    }

    glm::vec3 uniformHemisphere() {
        glm::vec3 result;
        float x1 = nextFloat(), x2 = nextFloat();
        float s = glm::sqrt(1.0f - x1 * x1);
        result.x = glm::cos(glm::two_pi<float>() * x2) * s;
        result.y = glm::sin(glm::two_pi<float>() * x2) * s;
        result.z = x1;
        return result;
    }

    glm::vec3 uniformCircle() {
        glm::vec3 result(0.0f);
        float x = nextFloat();
        result.x = glm::cos(glm::two_pi<float>() * x);
        result.y = glm::sin(glm::two_pi<float>() * x);
        return result;
    }
};





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


DeferredPhongPainter::DeferredPhongPainter(
    GBuffer gBuffer,
    Mesh* quadMesh,
    GLuint ssaoTexHandle,
    GLuint shadowMapTexHandle)
    : mInputGBuffer(gBuffer),
    mQuadMesh(quadMesh),
    mSSAOTextureHandle(ssaoTexHandle),
    mShadowMapTextureHandle(shadowMapTexHandle) {}

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
    glViewport(0, 0, 1024, 768);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // bind the textures of G-buffer, generated previously to the 
    // texture unit 0 , 1, 2 respectively, ready to be sampled by this Painter

    BindToTextureUnit(0, mInputGBuffer.mPositionTextureHandle);
    BindToTextureUnit(1, mInputGBuffer.mNormalTextureHandle);
    BindToTextureUnit(2, mInputGBuffer.mColorTextureHandle);
    BindToTextureUnit(3, mSSAOTextureHandle);
    BindToTextureUnit(4, mShadowMapTextureHandle);

    // calculate & send LightPosView;
    // todo: move this to a common method in Painter if can reuse

    glm::mat4 view = glm::lookAt(
        mStage->GetCamera().mPosition, // camera pos in world space
        mStage->GetCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::vec4 lightPosView = view * glm::vec4(mStage->GetLight()[0].mPositon, 1.0f);
    mShaderProgram.SendVec4Uniform("LightPosView", lightPosView);

    // calculate inverseMVThenLightMVP: Since G-buffer has the vertex position
    // in view space which is View * Model * VertexPosition. Now we want the vertex position
    // in Light space. So first we need to invert the view matrix, and then multiply with
    // ViewFromLight and LightPerspectiveProjection matrix.

    glm::mat4 lightView = glm::lookAt(
        mStage->GetLight()[0].mPositon, // camera pos in world space
        mStage->GetCamera().mTarget, // camera target
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    //todo: hardcode value but should do dynamic calculation here
    glm::mat4 lightProj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 14.0f, 22.0f);

    glm::mat4 inverseVThenLightMVP = lightProj * lightView * glm::inverse(view);

    mShaderProgram.SendMat4Uniform("inverseVThenLightMVP", inverseVThenLightMVP);


    mQuadMesh->BindBuffers();

    RenderMesh(mQuadMesh);

}

void TexturePainter::DrawObjects()
{
    mShaderProgram.MakeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    BindToTextureUnit(0, mInputTextureHandle);

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
: mInputTextureHandle(textureHandle), mQuadMesh(quadMesh) {}

TexturePainter::~TexturePainter() {}


void SSAOPainter::Init()
{
    // load shader
    mShaderProgram.LoadShader("quadVert", "fullScreenQuad.vert.glsl", GL_VERTEX_SHADER);
    mShaderProgram.LoadShader("ssaoFrag", "ssao.frag.glsl", GL_FRAGMENT_SHADER);
    mShaderProgram.CreateProgram("quadVert", "ssaoFrag");

    // create a texture and FBO to bind to
    glGenFramebuffers(1, &mFrameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferHandle);

    // create the texture to store AO value
    glGenTextures(1, &mSSAOTextureHandle);
    glBindTexture(GL_TEXTURE_2D, mSSAOTextureHandle);
    // Note the use of GL_RED here. AO is a single float value, so just GL_RED, not GL_RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 768, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mSSAOTextureHandle, 0);

    // - tell OpenGL the mapping from fragment shader out variables to this framebuffer
   // set the output "layout (location = 0) out ...;" in fragment shader to be render to
   // GL_COLOR_ATTACHMENT0 of frame buffer, which is bind to the texture mSSAOTextureHandle
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    // set the attachment to the currently bind frame buffer
    // note: this is different from glDrawBuffer (without s at the end)
    glDrawBuffers(1, attachments);

    // good practice to bind back to default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mShaderProgram.MakeCurrent();
    std::vector<glm::vec3> randoms = GenerateRandomVectors(0.09, 32);
    GLuint modelLoc = glGetUniformLocation(mShaderProgram.GetProgramHandle(), "gKernel");
    glUniform3fv(modelLoc, randoms.size(), glm::value_ptr(randoms[0]));

    //std::vector<glm::vec3> noise = GenerateRandomVectors(1.0, 16);
    // test
    Random rand;
    int size = 4;
    std::vector<GLfloat> randDirections(3 * size * size);
    for (int i = 0; i < size * size; i++) {
        glm::vec3 v = rand.uniformCircle();
        randDirections[i * 3 + 0] = v.x;
        randDirections[i * 3 + 1] = v.y;
        randDirections[i * 3 + 2] = v.z;
    }

    // generate noise texture
    glGenTextures(1, &noiseTextureHandle);
    glBindTexture(GL_TEXTURE_2D, noiseTextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, randDirections.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

std::vector<glm::vec3> SSAOPainter::GenerateRandomVectors(float maxLength, int size)
{
    // generate random vectors to be added to fragment position in view space.
    std::vector<glm::vec3> randomVectors(size);

    for (int i = 0; i < size; i++)
    {
        randomVectors[i] = glm::vec3(
            float(rand() % 100 + 10) / 100.0f * maxLength * (rand() % 100 > 49 ? 1.0 : -1.0 ),
            float(rand() % 100 + 10) / 100.0f * maxLength * (rand() % 100 > 49 ? 1.0 : -1.0),
            float(rand() % 100 + 10) / 100.0f * maxLength * (rand() % 100 > 49 ? 1.0 : -1.0));
    }
    
    return randomVectors;

}

void SSAOPainter::DrawObjects()
{
    mShaderProgram.MakeCurrent();
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferHandle);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // bind the textures of G-buffer, generated previously to the 
    // texture unit 0. This SSAO implementation only use view position data

    BindToTextureUnit(0, mInputGBuffer.mPositionTextureHandle);
    BindToTextureUnit(1, mInputGBuffer.mNormalTextureHandle);
    BindToTextureUnit(2, noiseTextureHandle);

    // send random vectors:

    // send projection matrix
    mShaderProgram.SendMat4Uniform("projectionMatrix", mStage->GetProjectionMatrix());

    mQuadMesh->BindBuffers();
    RenderMesh(mQuadMesh);
}

SSAOPainter::SSAOPainter(GBuffer gBuffer, Mesh* quadMesh)
    : mInputGBuffer(gBuffer), mQuadMesh(quadMesh) {}

SSAOPainter::~SSAOPainter(){}

GLuint SSAOPainter::GetSSAOTextureHandle()
{
    return mSSAOTextureHandle;
}
