#ifndef IMG_RESOURCEDATA_H
#define IMG_RESOURCEDATA_H

#include "img/Buffer.h"
#include "img/BlendMode.h"
namespace img { class ResourceNode; }

namespace img
{

class ResourceData
{
public:
    ResourceData(const QString& aIdentifier, const ResourceNode* aSerialAddress);
    virtual ~ResourceData() {}

    void grabImage(const XCMemBlock& aBlock, const QSize& aSize, Format aFormat);
    XCMemBlock releaseImage();

    void setPos(const QPoint& aPos);
    void setUserData(void* aData) { mUserData = aData; }
    void setIsLayer(bool aIsLayer) { mIsLayer = aIsLayer; }
    void setBlendMode(BlendMode aMode);

    bool isLayer() const { return mIsLayer; }
    bool hasImage() const { return mBuffer.data(); }
    const QString& identifier() const { return mIdentifier; }
    const img::Buffer& image() const { return mBuffer; }
    const QPoint& pos() const { return mPos; }
    void* userData() const { return mUserData; }
    BlendMode blendMode() const { return mBlendMode; }

    const ResourceNode* serialAddress() const { return mSerialAddress; }

private:
    img::Buffer mBuffer;
    QPoint mPos;
    void* mUserData;
    bool mIsLayer;
    QString mIdentifier;
    BlendMode mBlendMode;
    const ResourceNode* mSerialAddress;
};

} // namespace img

#endif // IMG_RESOURCEDATA_H
