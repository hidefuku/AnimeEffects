#ifndef CORE_BONEKEYUPDATER_H
#define CORE_BONEKEYUPDATER_H

#include "core/TimeLine.h"
#include "core/ObjectTreeEvent.h"
#include "core/ResourceEvent.h"
#include "core/TimeLineEvent.h"
#include "core/ProjectEvent.h"
namespace core { class ObjectTree; }

namespace core
{

class BoneKeyUpdater
{
public:
    static void onTimeLineModified(TimeLineEvent& aEvent);

    static void onTreeRestructured(ObjectTreeEvent& aEvent);

    static void onResourceModified(ResourceEvent& aEvent);

    static void onProjectAttributeModified(ProjectEvent& aEvent);

     // for tree restructuring
    static cmnd::Base* createNodeUnbinderForDelete(ObjectNode& aNode);
    static cmnd::Base* createNodeUnbinderForMove(
            ObjectTree& aTree, const util::TreePos& aFrom, const util::TreePos& aTo);

private:
    static void onTimeLineModified(
            Project& aProject, ObjectNode& aNode,
            const QVector<ObjectNode*>& aUniqueRoots,
            bool aResetCacheList);

    static void resetInfluenceCachesOfChildren(Project& aProject, ObjectNode& aRoot);
    static void resetInfluenceCachesOfOneNode(Project& aProject, ObjectNode& aNode);
};

} // namespace core

#endif // CORE_BONEKEYUPDATER_H
