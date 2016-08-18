#include "img/Buffer.h"

namespace
{

int getChannelNum(img::Format aFormat)
{
    switch (aFormat)
    {
    case img::Format_RGBA8:
        return 4;
    case img::Format_RGB8:
        return 3;
    case img::Format_A8:
        return 1;
    default:
        XC_ASSERT(0);
        return 1;
    }
}

}

namespace img
{

Buffer::Buffer()
    : mFormat(Format_TERM)
    , mBlock()
    , mWidth()
    , mHeight()
    , mChannelNum()
{
}

Buffer::~Buffer()
{
    free();
}

Buffer::Buffer(const Buffer& aRhs)
    : mFormat(aRhs.mFormat)
    , mBlock()
    , mWidth(aRhs.mWidth)
    , mHeight(aRhs.mHeight)
    , mChannelNum(aRhs.mChannelNum)
{
    if (aRhs.mBlock.data)
    {
        alloc(aRhs.mFormat, QSize(aRhs.mWidth, aRhs.mHeight));
        memcpy(mBlock.data, aRhs.mBlock.data, aRhs.mBlock.size);
    }
}

Buffer& Buffer::operator=(const Buffer& aRhs)
{
    free();

    mFormat = aRhs.mFormat;
    mWidth = aRhs.mWidth;
    mHeight = aRhs.mHeight;
    mChannelNum = aRhs.mChannelNum;

    if (aRhs.mBlock.data)
    {
        alloc(aRhs.mFormat, QSize(aRhs.mWidth, aRhs.mHeight));
        memcpy(mBlock.data, aRhs.mBlock.data, aRhs.mBlock.size);
    }
    return *this;
}

void Buffer::grab(Format aFormat, const XCMemBlock& aBlock, const QSize& aSize)
{
    free();
    mFormat = aFormat;
    mBlock = aBlock;
    mWidth = aSize.width();
    mHeight = aSize.height();
    mChannelNum = getChannelNum(mFormat);
}

void Buffer::grab(Buffer& aUngrab)
{
    mFormat = aUngrab.mFormat;
    mBlock = aUngrab.mBlock;
    mWidth = aUngrab.mWidth;
    mHeight = aUngrab.mHeight;
    mChannelNum = aUngrab.mChannelNum;

    aUngrab.mBlock = XCMemBlock();
    aUngrab.mWidth = 0;
    aUngrab.mHeight = 0;
    aUngrab.mChannelNum = 0;
}

XCMemBlock Buffer::release()
{
    XCMemBlock block = mBlock;
    mBlock = XCMemBlock();
    mWidth = 0;
    mHeight = 0;
    mChannelNum = 0;
    mFormat = Format_TERM;

    return block;
}

void Buffer::alloc(Format aFormat, const QSize& aSize)
{
    free();
    mFormat = aFormat;
    mChannelNum = getChannelNum(mFormat);
    mWidth = aSize.width();
    mHeight = aSize.height();

    mBlock.size = mChannelNum * mWidth * mHeight;
    mBlock.data = new uint8[mBlock.size];
}

void Buffer::free()
{
    if (mBlock.data)
    {
        delete[] mBlock.data;
        mBlock = XCMemBlock();
        mWidth = 0;
        mHeight = 0;
        mChannelNum = 0;
        mFormat = Format_TERM;
    }
}

} // namespace img
