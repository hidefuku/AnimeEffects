#ifndef IMG_RESOURCEHANDLE_H
#define IMG_RESOURCEHANDLE_H

#include <memory>
#include "img/Buffer.h"
#include "img/BlendMode.h"
namespace img { class ResourceNode; }

namespace img
{

#if 0
class ResourceHandle
{
public:
    ResourceHandle();
    ResourceHandle(ResourceNode& aNode);
    ResourceHandle(const ResourceHandle& aRhs);
    ~ResourceHandle();
    ResourceHandle& operator =(const ResourceHandle& aRhs);
    explicit operator bool() const { return mNode; }
    bool hasImage() const;
    const QString& identifier() const;
    const img::Buffer& image() const;
    QPoint pos() const;
    BlendMode blendMode() const;

    const ResourceNode* serialAddress() const { return mNode; }

private:
    void inc();
    void dec();
    ResourceNode* mNode;
};
#else

class ResourceData
{
public:
    ResourceData(const QString& aIdentifier, const void* aSerialAddress);
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

    const void* serialAddress() const { return mSerialAddress; }

private:
    img::Buffer mBuffer;
    QPoint mPos;
    void* mUserData;
    bool mIsLayer;
    QString mIdentifier;
    BlendMode mBlendMode;
    const void* mSerialAddress;
};

typedef std::shared_ptr<ResourceData> ResourceHandle;

#endif

} // namespace img

#endif // IMG_RESOURCEHANDLE_H
