#pragma once
#include "Painter.h"
#include <random>
struct GBuffer
{
    GLuint mPositionTextureHandle;
    GLuint mNormalTextureHandle;
    GLuint mColorTextureHandle;
};

// A painter who builds the G-Buffer
class GBufferPainter : public Painter
{
private:
    // handle to frame buffer(G-buffer)
    GLuint mGBufferFrameBufferHandle;
    // texture handles for each component
    GLuint mGPositionTexHandle, mGNormalTexHandle, mGColorTexHandle;
public:
    void DrawObjects();
    void Init();

    GBuffer GetGBuffer();

    GBufferPainter();
    ~GBufferPainter();
};

// todo: this painter actually doesn't use its own mObjects to render.
// it just renders a quad mesh and pull data from the G-buffer.
// Separate this somehow? Since assignObject and assignStage doesn't make sense
// with this painter anymore? Good analogy is a collager
class DeferredPhongPainter : public Painter
{
private:
    // handle to frame buffer(G-buffer)
    GBuffer mInputGBuffer;
    Mesh* mQuadMesh;
    GLuint mSSAOTextureHandle;
    GLuint mShadowMapTextureHandle;
    
public:
    void DrawObjects();
    void Init();
    
    // This Phong painter needs a GBuffer, and a quad mesh to paint
    DeferredPhongPainter(
        GBuffer gBuffer,
        Mesh* quadMesh,
        GLuint ssaoTexHandle,
        GLuint shadowMapTexHandle);
    ~DeferredPhongPainter();
};

// This class renders a texture to full screen quad. Useful for debugging
// deferred shading pass.
class TexturePainter : public Painter
{
private:
    GLuint mInputTextureHandle;
    Mesh* mQuadMesh;
public:

    void DrawObjects();
    void Init();

    // Give me a quad mesh and a texture, I'll render the full screen quad with the texture
    TexturePainter(GLuint textureHandle, Mesh* quadMesh);
    ~TexturePainter();
};

// This class use a G-Buffer to render a texture with ambient occlusion
// component
class SSAOPainter : public Painter
{
private:
    GBuffer mInputGBuffer;
    Mesh* mQuadMesh;
    
    GLuint mSSAOTextureHandle; // This painter draws the AO values to this texture
    GLuint mFrameBufferHandle;

    GLuint noiseTextureHandle;

    std::vector<glm::vec3> GenerateRandomVectors(float maxLength, int size);
public:

    void DrawObjects();
    void Init();

    // Give me a G buffer, I'll render the AO value to another texture(which I will create and
    // return to you
    SSAOPainter(GBuffer gBuffer, Mesh* quadMesh);
    ~SSAOPainter();

    GLuint GetSSAOTextureHandle();
};
