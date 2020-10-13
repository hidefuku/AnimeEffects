#ifndef GL_FONTDRAWER_H
#define GL_FONTDRAWER_H

#include <QFont>
#include <QPainterPath>
#include "gl/Framebuffer.h"
#include "gl/Texture.h"
#include "gl/BufferObject.h"
#include "gl/EasyShaderProgram.h"
#include "gl/TextObject.h"
#include "gl/EasyTextureDrawer.h"

namespace gl
{

class FontDrawer
{
public:
    FontDrawer();
    void setColor(const QColor& aColor);
    void draw(const QFont& aFont, TextObject& aTextObj);

private:
    bool initShader();
    void updateWorkTextureCache(TextObject& aTextObj, const QSize& aNeedSize);
    void createCascadePolygons(const QFont& aFont, const QFontMetrics& aMetrics,
                               const QString& aText, QVector<gl::Vector2>& aDest);

    gl::Framebuffer mFramebuffer;
    gl::EasyShaderProgram mShader;
    gl::EasyTextureDrawer mResizer;
    QColor mColor;
    int mLocPosition;
    int mLocViewMtx;
    TextObject::WorkCache mTextureCache;
};

} // namespace gl

#endif // GL_FONTDRAWER_H
