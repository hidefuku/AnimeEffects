#include "core/TimeLineEvent.h"
#include "core/ObjectNode.h"

namespace core
{

TimeLineEvent::TimeLineEvent()
    : mProject(nullptr)
    , mType(Type_TERM)
    , mTargets()
    , mDefaultTargets()
{
}

void TimeLineEvent::pushTarget(ObjectNode& aNode, const TimeKeyPos& aPos)
{
    Target target(aNode, aPos, aPos.index());
    mTargets.push_back(target);
}

void TimeLineEvent::pushTarget(ObjectNode& aNode, const TimeKeyPos& aPos, int aSubIndex)
{
    Target target(aNode, aPos, aSubIndex);
    mTargets.push_back(target);
}

void TimeLineEvent::pushTarget(ObjectNode& aNode, TimeKeyType aType, int aIndex)
{
    XC_PTR_ASSERT(aNode.timeLine());
    Target target(aNode, TimeKeyPos(*aNode.timeLine(), aType, aIndex), aIndex);
    mTargets.push_back(target);
}

void TimeLineEvent::pushTarget(ObjectNode& aNode, TimeKeyType aType, int aIndex, int aSubIndex)
{
    XC_PTR_ASSERT(aNode.timeLine());
    Target target(aNode, TimeKeyPos(*aNode.timeLine(), aType, aIndex), aSubIndex);
    mTargets.push_back(target);
}

void TimeLineEvent::pushDefaultTarget(ObjectNode& aNode, TimeKeyType aType)
{
    XC_PTR_ASSERT(aNode.timeLine());
    Target target(aNode, TimeKeyPos(*aNode.timeLine(), aType, TimeLine::kDefaultKeyIndex), TimeLine::kDefaultKeyIndex);
    mDefaultTargets.push_back(target);
}

} // namespace core
