#ifndef IMG_RESOURCEHANDLE_H
#define IMG_RESOURCEHANDLE_H

#include <memory>
#include "img/ResourceData.h"

namespace img
{

class ResourceHandle
{
public:
    ResourceHandle();
    ~ResourceHandle();
    ResourceHandle(ResourceData* aData, int* aOriginKeepingCount);
    ResourceHandle(const ResourceHandle& aRhs);
    ResourceHandle& operator=(const ResourceHandle& aRhs);

    explicit operator bool() const { return (bool)mData; }
    ResourceData* operator->() const { return mData.get(); }
    ResourceData& operator*() const { return *mData; }
    ResourceData* get() const { return mData.get(); }
    void reset();

    void swapData(ResourceHandle& aRhs);

    int referenceCount() const { return mData.use_count(); }
    int originKeepingCount() const { return *mOriginKeepingCount; }

    void setOriginKeeping(bool aKeepOrigin);

private:
    std::shared_ptr<ResourceData> mData;
    std::shared_ptr<int> mOriginKeepingCount;
    bool mKeepOrigin;
};
} // namespace img

#endif // IMG_RESOURCEHANDLE_H
