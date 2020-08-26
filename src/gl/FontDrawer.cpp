#include "gl/FontDrawer.h"
#include "gl/Global.h"
#include "gl/Util.h"

#include <QPainterPath>

namespace
{
static const int kAttachmentId = 0;
}

namespace gl
{

FontDrawer::FontDrawer()
    : mFramebuffer()
    , mShader()
    , mResizer()
    , mColor()
    , mLocPosition(-1)
    , mLocViewMtx(-1)
    , mTextureCache()
{
    initShader();
    mResizer.init();
}

bool FontDrawer::initShader()
{
    static const char* kVertexShaderText =
            "#version 330 \n"
            "in vec2 inPosition;"
            "uniform mat4 uViewMtx;"
            "void main(void){"
            "  gl_Position = uViewMtx * vec4(inPosition, 0.0, 1.0);"
            "}";
    static const char* kFragmentShaderText =
            "#version 330 \n"
            "layout(location = 0, index = 0) out vec4 oFragColor;"
            "void main(void){"
            "  oFragColor = vec4(1);"
            "}";

    if (!mShader.setVertexSource(QString(kVertexShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile vertex shader.",
                       mShader.log());
        return false;
    }
    if (!mShader.setFragmentSource(QString(kFragmentShaderText)))
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to compile fragment shader.",
                       mShader.log());
        return false;
    }
    if (!mShader.link())
    {
        XC_FATAL_ERROR("OpenGL Error", "Failed to link shader.",
                       mShader.log());
        return false;
    }

    mLocPosition = mShader.attributeLocation("inPosition");
    mLocViewMtx = mShader.uniformLocation("uViewMtx");

    GL_CHECK_ERROR();
    return true;

}

void FontDrawer::setColor(const QColor& aColor)
{
    mColor = aColor;
}

void FontDrawer::draw(const QFont& aFont, TextObject& aTextObj)
{
    QFontMetrics metrics(aFont);
    const QRect boundingRect = metrics.boundingRect(aTextObj.text());
    //const QSize textureSize(boundingRect.right(), boundingRect.bottom());
    const QSize textureSize(boundingRect.size());
    //qDebug() << boundingRect;
    const QSize workSize = textureSize * 2;

    // reserve texture
    if (aTextObj.texture().size() != textureSize)
    {
        aTextObj.texture().create(textureSize);
        aTextObj.texture().setFilter(GL_LINEAR);
        aTextObj.texture().setWrap(GL_CLAMP_TO_BORDER, QColor(0, 0, 0, 0));
    }

    // reserve texture for workspace
    updateWorkTextureCache(aTextObj, workSize);

    auto& ggl = gl::Global::functions();

    // store previous state
    GLint prevFramebuffer = 0;
    GLint prevViewport[4] = {};
    {
        ggl.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
        XC_ASSERT(prevFramebuffer >= 0);
        ggl.glGetIntegerv(GL_VIEWPORT, prevViewport);
    }

    // attach a work texture to the framebuffer
    mFramebuffer.setColorAttachment(kAttachmentId, mTextureCache->id());
    XC_ASSERT(mFramebuffer.isComplete());

    mFramebuffer.bind();

    // setup draw buffers
    {
        const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
        ggl.glDrawBuffers(1, attachments);
    }

    // set viewport
    gl::Util::setViewportAsActualPixels(workSize);
    // clear texture
    gl::Util::clearColorBuffer(mColor.redF(), mColor.greenF(), mColor.blueF(), 0.0f);

    // make cascade polygons
    QVector<gl::Vector2> cascades;
    createCascadePolygons(aFont, metrics, aTextObj.text(), cascades);

    // draw cascade polygons with odd even fill
    {
        // provide odd even fill with alpha blending
        ggl.glEnable(GL_BLEND);
        ggl.glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ZERO);

        QMatrix4x4 viewMtx;
        viewMtx.ortho(0.0f, textureSize.width(), textureSize.height(), 0.0f, -1.0f, 1.0f);

        mShader.bind();
        mShader.setAttributeArray(mLocPosition, cascades.data(), cascades.size());
        mShader.setUniformValue(mLocViewMtx, viewMtx);
        ggl.glDrawArrays(GL_TRIANGLES, 0, cascades.size());
        mShader.release();

        ggl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    mFramebuffer.release();

    // resize texture
    {
        mFramebuffer.setColorAttachment(kAttachmentId, aTextObj.texture().id());
        XC_ASSERT(mFramebuffer.isComplete());

        mFramebuffer.bind();

        const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
        ggl.glDrawBuffers(1, attachments);

        gl::Util::setViewportAsActualPixels(textureSize);
        const GLuint clearColorU[] = {
            (GLuint)mColor.red(), (GLuint)mColor.green(), (GLuint)mColor.blue(), 255
        };
        ggl.glClearBufferuiv(GL_COLOR, kAttachmentId, clearColorU);

        mResizer.draw(mTextureCache->id(),
                      QRect(QPoint(0.0f, 0.0f), textureSize), textureSize,
                      QRect(QPoint(0.0f, 0.0f), workSize), mTextureCache->size());

        mFramebuffer.release();
    }

    // revert previous state
    {
        ggl.glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prevFramebuffer);
        ggl.glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    }
}

void FontDrawer::updateWorkTextureCache(TextObject& aTextObj, const QSize& aNeedSize)
{
    TextObject::WorkCache caches[2];
    int score[2] = { 0, 0 };
    caches[0].swap(mTextureCache);
    caches[1].swap(aTextObj.workCache());

    for (int i = 0; i < 2; ++i)
    {
        if (!caches[i]) continue;

        auto cacheSize = caches[i]->size();
        if (aNeedSize.width() <= cacheSize.width() && aNeedSize.height() < cacheSize.height())
        {
            const int cacheLength = cacheSize.width() * cacheSize.height();
            const int needLength = aNeedSize.width() * aNeedSize.height();

            if (cacheLength < needLength * 2)
            {
                score[i] = needLength * 2 - cacheLength;
            }
            // note: delete texture caches which too large
        }
    }

    if (score[0] == 0 && score[1] == 0)
    {
        caches[0].reset();
        caches[1].reset();

        // create new cache
        auto newCache = new gl::Texture();
        newCache->create(aNeedSize);
        newCache->setFilter(GL_LINEAR);
        newCache->setWrap(GL_CLAMP_TO_BORDER, QColor(0, 0, 0, 0));

        aTextObj.workCache().reset(newCache);
        mTextureCache = aTextObj.workCache();
    }
    else if (score[0] > score[1])
    {
        mTextureCache.swap(caches[0]);
        aTextObj.workCache() = mTextureCache;
    }
    else
    {
        aTextObj.workCache().swap(caches[1]);
        mTextureCache = aTextObj.workCache();
    }
}

void FontDrawer::createCascadePolygons(
        const QFont& aFont, const QFontMetrics& aMetrics,
        const QString& aText, QVector<gl::Vector2>& aDest)
{
    QPainterPath path;
    path.addText(0.0f, aMetrics.ascent(), aFont, aText);
    auto polygons = path.toSubpathPolygons();

    // get count of segments
    int segCount = 0;
    for (auto& polygon : polygons)
    {
        segCount += polygon.size();
    }

    aDest.resize(segCount * 6);

    int i = 0;
    for (auto& polygon : polygons)
    {
        if (polygon.empty()) continue;
        auto pre = polygon.back();
        for (auto vtx : polygon)
        {
            if (pre.x() < vtx.x())
            {
                aDest[i    ].set(pre.x(), pre.y());
                aDest[i + 1].set(vtx.x(), vtx.y());
                aDest[i + 2].set(pre.x(), 0.0f);
                aDest[i + 3].set(pre.x(), 0.0f);
                aDest[i + 4].set(vtx.x(), vtx.y());
                aDest[i + 5].set(vtx.x(), 0.0f);
            }
            else
            {
                aDest[i    ].set(vtx.x(), vtx.y());
                aDest[i + 1].set(pre.x(), pre.y());
                aDest[i + 2].set(pre.x(), 0.0f);
                aDest[i + 3].set(pre.x(), 0.0f);
                aDest[i + 4].set(vtx.x(), 0.0f);
                aDest[i + 5].set(vtx.x(), vtx.y());
            }
            pre = vtx;
            i += 6;
        }
    }
}

} // namespace gl
