#include "core/ResourceEvent.h"

namespace core
{

ResourceEvent::ResourceEvent(Project& aProject)
    : mProject(aProject)
    , mType(Type_Reload)
    , mRoot()
    , mTargets()
{
}

void ResourceEvent::setSingleTarget(img::ResourceNode& aNode)
{
    mRoot = &aNode;
    mTargets.clear();
    mTargets.push_back(&aNode);
}

bool ResourceEvent::contains(const void* aSerialAddress) const
{
    return mTargets.contains((const img::ResourceNode*)aSerialAddress);
}

const img::ResourceNode* ResourceEvent::findTarget(const void* aSerialAddress) const
{
    for (auto target : mTargets)
    {
        if (target == aSerialAddress) return target;
    }
    return nullptr;
}

} // namespace core

