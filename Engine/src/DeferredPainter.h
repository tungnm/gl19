#pragma once
#include "Painter.h"

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

    GBufferPainter();
    ~GBufferPainter();
};