#ifndef CORE_OBJECTTREEEVENT_H
#define CORE_OBJECTTREEEVENT_H

#include <QVector>
#include "core/ObjectNode.h"
namespace core { class Project; }

namespace core
{

class ObjectTreeEvent
{
public:
    enum Type
    {
        Type_Add,
        Type_Delete,
        Type_Move,
        Type_TERM
    };

    struct Target
    {
        Target();
        Target(ObjectNode* aParent, ObjectNode& aNode);
        ObjectNode* parent;
        ObjectNode* node;
    };

    ObjectTreeEvent(Project& aProject);

    void setType(Type aType);
    void pushTarget(ObjectNode* aParent, ObjectNode& aNode);

    Project& project() const { return mProject; }
    Type type() const { return mType; }
    QVector<Target>& targets() { return mTargets; }
    const QVector<Target>& targets() const { return mTargets; }

    // for ObjectTreeNotifier
    QVector<ObjectNode*>& roots() { return mRoots; }
    const QVector<ObjectNode*>& roots() const { return mRoots; }

private:
    Project& mProject;
    Type mType;
    QVector<Target> mTargets;
    QVector<ObjectNode*> mRoots;
};

} // namespace core

#endif // CORE_OBJECTTREEEVENT_H
