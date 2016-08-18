#include "core/ObjectTreeEvent.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
ObjectTreeEvent::Target::Target()
    : parent()
    , node()
{
}

ObjectTreeEvent::Target::Target(ObjectNode* aParent, ObjectNode& aNode)
    : parent(aParent)
    , node(&aNode)
{
}

//-------------------------------------------------------------------------------------------------
ObjectTreeEvent::ObjectTreeEvent(Project& aProject)
    : mProject(aProject)
    , mType(Type_TERM)
    , mTargets()
    , mRoots()
{
}

void ObjectTreeEvent::setType(Type aType)
{
    mType = aType;
}

void ObjectTreeEvent::pushTarget(ObjectNode* aParent, ObjectNode& aNode)
{
    mTargets.push_back(Target(aParent, aNode));
}

} // namespace core
