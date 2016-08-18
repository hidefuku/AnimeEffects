#ifndef CORE_BONEKEYUPDATER_H
#define CORE_BONEKEYUPDATER_H

#include "core/TimeLine.h"
#include "core/ObjectTreeEvent.h"
#include "core/ResourceEvent.h"
#include "core/TimeLineEvent.h"
#include "core/ProjectEvent.h"

namespace core
{

class BoneKeyUpdater
{
public:
    static void onTimeLineModified(TimeLineEvent& aEvent);

    static void onTreeRestructured(ObjectTreeEvent& aEvent);

    static void onResourceModified(ResourceEvent& aEvent);

    static void onProjectAttributeModified(ProjectEvent& aEvent);

private:
    static void onTimeLineModified(
            Project& aProject, ObjectNode& aNode,
            const QVector<ObjectNode*>& aUniqueRoots);

    static void resetInfluenceCaches(Project& aProject, ObjectNode& aRoot);
};

} // namespace core

#endif // CORE_BONEKEYUPDATER_H
