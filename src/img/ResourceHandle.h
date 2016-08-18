#ifndef IMG_RESOURCEHANDLE_H
#define IMG_RESOURCEHANDLE_H

#include "img/Buffer.h"
#include "img/BlendMode.h"
namespace img { class ResourceNode; }

namespace img
{

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

} // namespace img

#endif // IMG_RESOURCEHANDLE_H
