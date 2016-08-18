#ifndef IMG_BUFFER_H
#define IMG_BUFFER_H

#include <QColor>
#include <QSize>
#include "XC.h"
#include "img/Format.h"

namespace img
{

class Buffer
{
public:
    Buffer();
    Buffer(const Buffer& aRhs);
    Buffer& operator=(const Buffer& aRhs);
    ~Buffer();

    void grab(Format aFormat, const XCMemBlock& aBlock, const QSize& aSize);
    void grab(Buffer& aUngrab);
    XCMemBlock release();
    void alloc(Format aFormat, const QSize& aSize);
    void free();

    Format format() const { return mFormat; }
    uint8* data() { return mBlock.data; }
    const uint8* data() const { return mBlock.data; }
    size_t size() const { return mBlock.size; }
    const XCMemBlock& block() const { return mBlock; }

    int width() const { return mWidth; }
    int height() const { return mHeight; }
    QSize pixelSize() const { return QSize(mWidth, mHeight); }

    template<typename tChannel>
    inline tChannel* rawPixel(int aX, int aY) const
    {
        return (tChannel*)(mBlock.data + (aX + aY * mWidth) * mChannelNum);
    }

private:
    Format mFormat;
    XCMemBlock mBlock;
    int mWidth;
    int mHeight;
    int mChannelNum;
};

} // namespace img

#endif // IMG_BUFFER_H
