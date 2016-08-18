#ifndef CORE_TIMELINEEVENT
#define CORE_TIMELINEEVENT

#include <QVector>
#include "core/TimeKeyPos.h"
namespace core { class Project; }
namespace core { class ObjectNode; }

namespace core
{

class TimeLineEvent
{
public:
    enum Type
    {
        Type_PushKey,
        Type_RemoveKey,
        Type_MoveKey,
        Type_ChangeKeyValue,
        Type_TERM
    };

    struct Target
    {
        Target()
            : node(), pos(), subIndex() {}
        Target(ObjectNode& aNode, const TimeKeyPos& aPos, int aSubIndex)
            : node(&aNode), pos(aPos), subIndex(aSubIndex) {}

        ObjectNode* node;
        TimeKeyPos pos;
        int subIndex;
    };

    TimeLineEvent();

    void setType(Type aType) { mType = aType; }

    void pushTarget(ObjectNode& aNode, const TimeKeyPos& aPos);
    void pushTarget(ObjectNode& aNode, const TimeKeyPos& aPos, int aSubIndex);
    void pushTarget(ObjectNode& aNode, TimeKeyType aType, int aIndex);
    void pushTarget(ObjectNode& aNode, TimeKeyType aType, int aIndex, int aSubIndex);

    Type type() const { return mType; }
    QVector<Target>& targets() { return mTargets; }
    const QVector<Target>& targets() const { return mTargets; }

    // set a project by a project
    void setProject(Project& aProject) { mProject = &aProject; }
    Project& project() const { XC_PTR_ASSERT(mProject); return *mProject; }

private:
    Project* mProject;
    Type mType;
    QVector<Target> mTargets;
};

} // namespace core

#endif // CORE_TIMELINEEVENT

