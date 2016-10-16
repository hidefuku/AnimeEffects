#include "gl/Global.h"
#include "gl/Texture.h"

#define F_MAKE_MIPMAP 0

#if F_MAKE_MIPMAP
class PreMipmapImage
{
    void* mData;
    int mWidth;
    int mHeight;
public:
    struct Pixel { uint8 r, g, b, a; };

    PreMipmapImage(void* aImageData, int aWidth, int aHeight)
        : mData()
        , mWidth(aWidth)
        , mHeight(aHeight)
    {
        const size_t size = aWidth * aHeight * sizeof(Pixel);
        mData = std::malloc(size);
        std::memcpy(mData, aImageData, size);
        convert();
    }

    ~PreMipmapImage()
    {
        std::free(mData);
    }

    void* data() { return mData; }
    const void* data() const { return mData; }

private:
    void convert()
    {
        Pixel* pixels = (Pixel*)mData;
        const int w = mWidth - 1;
        const int h = mHeight - 1;
        const int v = mWidth;
        const int v1 = v + 1;

        for (int y = 0; y < h; ++y)
        {
            const int yv = y * v;

            for (int x = 0; x < w; ++x)
            {
                Pixel* p = &pixels[yv + x];
                const uint32 a = p->a;
                const uint32 asum = a + (p + 1)->a + (p + v)->a + (p + v1)->a;

                if (asum == 0) continue;

                const uint32 ax4 = std::min(a * 4, asum);
                p->r = (uint8)std::min(ax4 * p->r / asum, (uint32)0xff);
                p->g = (uint8)std::min(ax4 * p->g / asum, (uint32)0xff);
                p->b = (uint8)std::min(ax4 * p->b / asum, (uint32)0xff);
            }
        }
    }
};
#endif

namespace gl
{


Texture::Texture()
    : mId(0)
    , mSize()
{
}

Texture::~Texture()
{
    destroy();
}

void Texture::create(
        const QSize& aSize, const uint8* aData,
        GLenum aFormat, GLint aInternalFormat, GLenum aChannelType)
{
    destroy();

    XC_ASSERT(!aSize.isNull());

    mSize = aSize;

    Global::Functions& ggl = Global::functions();
    ggl.glGenTextures(1, &mId);
    ggl.glBindTexture(GL_TEXTURE_2D, mId);

#if F_MAKE_MIPMAP
    PreMipmapImage preMipmap(aData, mSize.width(), mSize.height());
    ggl.glTexImage2D(
                GL_TEXTURE_2D, 0, aInternalFormat, mSize.width(), mSize.height(),
                0, aFormat, aChannelType, (uint8*)preMipmap.data());
    ggl.glGenerateMipmap(GL_TEXTURE_2D);
    //ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    //ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
#else
    ggl.glTexImage2D(
                GL_TEXTURE_2D, 0, aInternalFormat, mSize.width(), mSize.height(),
                0, aFormat, aChannelType, aData);
#endif
    ggl.glBindTexture(GL_TEXTURE_2D, 0);

    XC_ASSERT(ggl.glGetError() == GL_NO_ERROR);
}

void Texture::setFilter(GLint aParam)
{
    Global::Functions& ggl = Global::functions();
    ggl.glBindTexture(GL_TEXTURE_2D, mId);
    ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aParam);
    ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, aParam);
    ggl.glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setWrap(GLint aParam, QColor aBorderColor)
{
    Global::Functions& ggl = Global::functions();
    ggl.glBindTexture(GL_TEXTURE_2D, mId);
    ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, aParam);
    ggl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, aParam);

    if (aParam == GL_CLAMP_TO_BORDER)
    {
        const GLfloat colorF[] = {
            (GLfloat)aBorderColor.redF(), (GLfloat)aBorderColor.greenF(),
            (GLfloat)aBorderColor.blueF(), (GLfloat)aBorderColor.alphaF() };

        ggl.glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, colorF);
    }
    ggl.glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::destroy()
{
    if (mId != 0)
    {
        Global::functions().glDeleteTextures(1, &mId);
        mId = 0;
        mSize = QSize();
        GL_CHECK_ERROR();
    }
}

} // namespace gl
