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
    enum Type
    {
        Type_AddTree,
        Type_Delete,
        Type_Reload,
        Type_Rename,
        Type_TERM
    };

    typedef QVector<const img::ResourceNode*> Targets;

    ResourceEvent(Project& aProject);

    void setType(Type aType)
    {
        mType = aType;
    }

    void setRoot(img::ResourceNode& aNode)
    {
        mRoot = &aNode;
    }

    void pushTarget(img::ResourceNode& aNode)
    {
        mTargets.push_back(&aNode);
    }

    void setSingleTarget(img::ResourceNode& aNode);

    Project& project() const { return mProject; }

    Type type() const { return mType; }

    img::ResourceNode* root() { return mRoot; }
    const img::ResourceNode* root() const { return mRoot; }

    Targets& targets() { return mTargets; }
    const Targets& targets() const { return mTargets; }

    bool contains(const void* aSerialAddress) const;
    const img::ResourceNode* findTarget(const void* aSerialAddress) const;

private:
    Project& mProject;
    Type mType;
    img::ResourceNode* mRoot;
    Targets mTargets;
};

} // namespace core

#endif // CORE_RESOURCEEVENT_H
