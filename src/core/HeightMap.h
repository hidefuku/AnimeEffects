#ifndef CORE_HEIGHTMAP_H
#define CORE_HEIGHTMAP_H

#include <QRect>
#include <QVector3D>
#include <QVector2D>
#include "XC.h"
#include "img/Buffer.h"

namespace core
{

class HeightMap
{
public:
    HeightMap(const QString& aName);

    void grabImage(const XCMemBlock& aBlock, const QRect& aRect);
    img::Buffer& image() { return mImage; }
    const img::Buffer& image() const { return mImage; }

    void setHeightRate(float aRate) { mHeightRate = aRate; }
    void setBaseHeight(float aBase) { mBaseHeight = aBase; }

    float readHeight(const QVector2D& aPos) const;

private:
    QString mName;
    img::Buffer mImage;
    QRect mImageRect;
    QVector3D mCenter;
    float mHeightRate;
    float mBaseHeight;
};

} // namespace core

#endif // CORE_HEIGHTMAP_H
