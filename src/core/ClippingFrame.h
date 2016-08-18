#ifndef CORE_CLIPPINGFRAME_H
#define CORE_CLIPPINGFRAME_H

#include <QScopedPointer>
#include "gl/Framebuffer.h"
#include "gl/Texture.h"
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"

namespace core
{

class ClippingFrame
{
public:
    ClippingFrame();

    void resize(const QSize& aSize);
    void clearTexture();
    void singulate(uint8 aId);
    //void drawClipping(gl::BufferObject& aPositions);
    void bind();
    void release();
    void setupDrawBuffers();

    void resetClippingId() { mClippingId = 0; }
    uint8 forwardClippingId();
    uint8 clippingId() const { return mClippingId; }

    gl::Texture& texture() { return *mTexture; }
    const gl::Texture& texture() const { return *mTexture; }

    uint32 renderStamp() const { return mRenderStamp; }
    void updateRenderStamp() { ++mRenderStamp; }

private:
    void createSingulationShader();

    QScopedPointer<gl::Framebuffer> mFramebuffer;
    QScopedPointer<gl::Texture> mTexture;
    uint8 mClippingId;
    gl::EasyShaderProgram mSingulationShader;
    uint32 mRenderStamp;
};

} // namespace core

#endif // CORE_CLIPPINGFRAME_H
