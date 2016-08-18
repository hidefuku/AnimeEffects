#include "util/TreeUtil.h"
#include "core/BoneKeyUpdater.h"
#include "core/BoneKey.h"
#include "core/Project.h"

namespace core
{

void BoneKeyUpdater::onTimeLineModified(TimeLineEvent& aEvent)
{
    // pass only srt key updating
    QVector<ObjectNode*> targets;
    for (auto t : aEvent.targets())
    {
        auto type = t.pos.type();
        if (type == TimeKeyType_SRT || type == TimeKeyType_Mesh)
        {
            targets.push_back(t.node);
        }
    }
    if (targets.empty()) return;

    // remove redundant nodes
    QVector<ObjectNode*> uniqueRoots = util::TreeUtil::getUniqueRoots(targets);

    for (auto root : uniqueRoots)
    {
        for (auto p = root; p; p = p->parent())
        {
            BoneKeyUpdater::onTimeLineModified(aEvent.project(), *p, uniqueRoots);
        }
    }
}

void BoneKeyUpdater::onTimeLineModified(
        Project& aProject, ObjectNode& aNode,
        const QVector<ObjectNode*>& aUniqueRoots)
{
    if (!aNode.timeLine()) return;
    auto& map = aNode.timeLine()->map(TimeKeyType_Bone);
    for (auto itr = map.begin(); itr != map.end(); ++itr)
    {
        TimeKey* key = itr.value();
        XC_PTR_ASSERT(key);
        XC_ASSERT(key->type() == TimeKeyType_Bone);
        ((BoneKey*)key)->updateCaches(aProject, aNode, aUniqueRoots);
    }
}

void BoneKeyUpdater::onTreeRestructured(ObjectTreeEvent& aEvent)
{
    for (ObjectNode* root : aEvent.roots())
    {
        XC_PTR_ASSERT(root);
        XC_ASSERT(root->canHoldChild());
        resetInfluenceCaches(aEvent.project(), *root);
    }
}

void BoneKeyUpdater::resetInfluenceCaches(Project& aProject, ObjectNode& aRoot)
{
    ObjectNode::Iterator itr(&aRoot);
    while (itr.hasNext())
    {
        auto node = itr.next();
        XC_PTR_ASSERT(node);

        if (node->timeLine())
        {
            auto& map = node->timeLine()->map(TimeKeyType_Bone);

            // update bone cache
            for (auto itr = map.begin(); itr != map.end(); ++itr)
            {
                TimeKey* key = itr.value();
                XC_PTR_ASSERT(key);
                XC_ASSERT(key->type() == TimeKeyType_Bone);
                ((BoneKey*)key)->resetCaches(aProject, *node);
            }
        }
    }
}

void BoneKeyUpdater::onResourceModified(ResourceEvent& aEvent)
{
    auto topNode = aEvent.project().objectTree().topNode();
    if (topNode)
    {
        resetInfluenceCaches(aEvent.project(), *topNode);
    }
}

void BoneKeyUpdater::onProjectAttributeModified(ProjectEvent& aEvent)
{
    auto topNode = aEvent.project().objectTree().topNode();
    if (topNode)
    {
        if (aEvent.type() == ProjectEvent::Type_ChangeMaxFrame ||
            aEvent.type() == ProjectEvent::Type_ChangeLoop)
        {
            resetInfluenceCaches(aEvent.project(), *topNode);
        }
    }
}

} // namespace core
