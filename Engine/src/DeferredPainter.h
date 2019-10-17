#pragma once
#include "Painter.h"
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

class DeferredPhongPainter : public Painter
{
private:
    // handle to frame buffer(G-buffer)
    GBuffer mGBuffer;
public:
    void DrawObjects();
    void Init();
    
    // This Phong painter needs a GBuffer to paint
    DeferredPhongPainter(GBuffer gBuffer);
    ~DeferredPhongPainter();
};