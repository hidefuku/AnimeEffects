#include "core/HeightMap.h"

namespace core
{

HeightMap::HeightMap(const QString& aName)
    : mName(aName)
    , mImage()
    , mImageRect()
    , mCenter()
    , mHeightRate(1.0f)
    , mBaseHeight(0.0f)
{
}

void HeightMap::grabImage(const XCMemBlock& aBlock, const QRect& aRect)
{
    mImage.free();
    mImage.grab(img::Format_RGBA8, aBlock, aRect.size());
    mImageRect = aRect;

    const QRectF rectF(aRect);
    mCenter = QVector3D(QVector2D(rectF.center()), 0.0f);
}

float HeightMap::readHeight(const QVector2D &aPos) const
{
    QVector2D minPos(mImageRect.topLeft());
    QVector2D pixelPos = aPos - minPos;
    int x = (int)pixelPos.x();
    int y = (int)pixelPos.y();

    if (x < 0 || mImage.width() <= x)
    {
        return 0.0f;
    }
    else if (y < 0 || mImage.height() <= y)
    {
        return 0.0f;
    }

    uint8* px = mImage.rawPixel<uint8>(x, y);
    float height = (float)(px[0] + px[1] + px[2]) / (3.0f * 255.0f);
    return mBaseHeight + mHeightRate * height;
}

} // namespace core
