#ifndef CORE_RESOURCEEVENT_H
#define CORE_RESOURCEEVENT_H

#include <QVector>
#include "img/ResourceNode.h"
namespace core { class Project; }

namespace core
{

class ResourceEvent
{
public:
    typedef QVector<const img::ResourceNode*> Targets;

    ResourceEvent(Project& aProject);

    void setRoot(img::ResourceNode& aNode)
    {
        mRoot = &aNode;
    }

    void pushTarget(img::ResourceNode& aNode)
    {
        mTargets.push_back(&aNode);
    }

    Project& project() const { return mProject; }

    img::ResourceNode* root() { return mRoot; }
    const img::ResourceNode* root() const { return mRoot; }

    Targets& targets() { return mTargets; }
    const Targets& targets() const { return mTargets; }

    bool contains(const void* aSerialAddress) const
    {
        return mTargets.contains((const img::ResourceNode*)aSerialAddress);
    }

    const img::ResourceNode* findTarget(const void* aSerialAddress) const
    {
        for (auto target : mTargets)
        {
            if (target == aSerialAddress) return target;
        }
        return nullptr;
    }

private:
    Project& mProject;
    img::ResourceNode* mRoot;
    Targets mTargets;
};

} // namespace core

#endif // CORE_RESOURCEEVENT_H
