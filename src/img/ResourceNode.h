#ifndef IMG_RESOURCENODE_H
#define IMG_RESOURCENODE_H

#include <QRect>
#include "util/TreeNodeBase.h"
#include "util/NonCopyable.h"
#include "img/Format.h"
#include "img/Buffer.h"
#include "img/ResourceHandle.h"
#include "img/BlendMode.h"

namespace img
{

class ResourceNode
        : public util::TreeNodeBase<ResourceNode>
        , private util::NonCopyable
{
public:
    ResourceNode(const QString& aIdentifier);
    virtual ~ResourceNode();

    void grabImage(const XCMemBlock& aBlock, const QSize& aSize, Format aFormat);
    XCMemBlock releaseImage();

    void setPos(const QPoint& aPos);
    void setUserData(void* aData) { mUserData = aData; }
    void setIsLayer(bool aIsLayer) { mIsLayer = aIsLayer; }
    void setBlendMode(BlendMode aMode);

    ResourceHandle handle();

    bool isLayer() const { return mIsLayer; }
    bool isReferenced() const { return mRefCount > 0; }
    bool hasImage() const { return mBuffer.data(); }
    const QString& identifier() const { return mIdentifier; }
    const img::Buffer& image() const { return mBuffer; }
    const QPoint& pos() const { return mPos; }
    void* userData() const { return mUserData; }
    BlendMode blendMode() const { return mBlendMode; }

    // util
    int getCountOfSameSiblings() const;

private:
    friend class ResourceHandle;
    void incrementRC();
    void decrementRC();

private:
    img::Buffer mBuffer;
    QPoint mPos;
    void* mUserData;
    bool mIsLayer;
    QString mIdentifier;
    BlendMode mBlendMode;
    int mRefCount;
};

} // namespace img

#endif // IMG_RESOURCENODE_H
