#include "util/TreeUtil.h"
#include "cmnd/Stable.h"
#include "core/BoneKeyUpdater.h"
#include "core/BoneKey.h"
#include "core/Project.h"

namespace core
{

void BoneKeyUpdater::onTimeLineModified(TimeLineEvent& aEvent)
{
    // pass only srt and mesh key(which affect to influence map)
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
    QVector<ObjectNode*> parents;

    for (ObjectNode* root : aEvent.roots())
    {
        XC_PTR_ASSERT(root);
        XC_ASSERT(root->canHoldChild());
        resetInfluenceCachesOfChildren(aEvent.project(), *root);

        for (ObjectNode* parent = root->parent(); parent; parent = parent->parent())
        {
            if (!parents.contains(parent))
            {
                parents.push_back(parent);
            }
        }
    }

    for (auto p : parents)
    {
        resetInfluenceCachesOfOneNode(aEvent.project(), *p);
    }
}

void BoneKeyUpdater::resetInfluenceCachesOfOneNode(Project& aProject, ObjectNode& aNode)
{
    if (aNode.timeLine())
    {
        auto& map = aNode.timeLine()->map(TimeKeyType_Bone);

        // update bone cache
        for (auto itr = map.begin(); itr != map.end(); ++itr)
        {
            TimeKey* key = itr.value();
            XC_PTR_ASSERT(key);
            XC_ASSERT(key->type() == TimeKeyType_Bone);
            ((BoneKey*)key)->resetCaches(aProject, aNode);
        }
    }
}

void BoneKeyUpdater::resetInfluenceCachesOfChildren(Project& aProject, ObjectNode& aRoot)
{
    ObjectNode::Iterator itr(&aRoot);
    while (itr.hasNext())
    {
        auto node = itr.next();
        XC_PTR_ASSERT(node);
        resetInfluenceCachesOfOneNode(aProject, *node);
    }
}

void BoneKeyUpdater::onResourceModified(ResourceEvent& aEvent)
{
    auto topNode = aEvent.project().objectTree().topNode();
    if (topNode)
    {
        resetInfluenceCachesOfChildren(aEvent.project(), *topNode);
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
            resetInfluenceCachesOfChildren(aEvent.project(), *topNode);
        }
    }
}

//-------------------------------------------------------------------------------------------------
class Unbinder
{
    struct Pos
    {
        Bone2* bone;
        int index;
    };

    ObjectNode* mNode;
    QList<Pos> mPositions;
public:
    Unbinder()
        : mNode()
        , mPositions()
    {
    }

    void initNode(ObjectNode& aNode)
    {
        mNode = &aNode;
    }

    bool add(Bone2& aBone)
    {
        const int index = aBone.bindingNodes().indexOf(mNode);
        if (index < 0) return false;
        Pos pos = { &aBone, index };
        mPositions.push_back(pos);
        return true;
    }

    void addAll(BoneKey& aKey)
    {
        for (auto topBone : aKey.data().topBones())
        {
            Bone2::Iterator boneItr(topBone);
            while (boneItr.hasNext())
            {
                add(*boneItr.next());
            }
        }
    }

    void exec()
    {
        for (auto pos : mPositions)
        {
            pos.bone->bindingNodes().removeAt(pos.index);
        }
    }

    void undo()
    {
        for (auto pos : mPositions)
        {
            pos.bone->bindingNodes().insert(pos.index, mNode);
        }
    }
};

cmnd::Base* BoneKeyUpdater::createNodeUnbinderForDelete(ObjectNode& aNode)
{
    class UnbinderForDelete : public cmnd::Stable
    {
        ObjectNode& mNode;
        Unbinder mUnbinder;
    public:
        UnbinderForDelete(ObjectNode& aNode)
            : mNode(aNode)
            , mUnbinder()
        {
            mUnbinder.initNode(aNode);
        }

        virtual void exec()
        {
            for (ObjectNode* node = mNode.parent(); node; node = node->parent())
            {
                if (!node->timeLine()) continue;

                auto& map = node->timeLine()->map(TimeKeyType_Bone);

                for (auto itr = map.begin(); itr != map.end(); ++itr)
                {
                    TimeKey* key = itr.value();
                    XC_PTR_ASSERT(key);
                    XC_ASSERT(key->type() == TimeKeyType_Bone);
                    mUnbinder.addAll(*((BoneKey*)key));
                }
            }
            redo();
        }

        virtual void redo()
        {
            mUnbinder.exec();
        }

        virtual void undo()
        {
            mUnbinder.undo();
        }
    };

    return new UnbinderForDelete(aNode);
}

cmnd::Base* BoneKeyUpdater::createNodeUnbinderForMove(
        ObjectTree& aTree, const util::TreePos& aFrom, const util::TreePos& aTo)
{
    class UnbinderForMove : public cmnd::Stable
    {
        ObjectTree& mTree;
        const util::TreePos mFrom;
        const util::TreePos mTo;
        Unbinder mUnbinder;
    public:
        UnbinderForMove(ObjectTree& aTree, const util::TreePos& aFrom, const util::TreePos& aTo)
            : mTree(aTree)
            , mFrom(aFrom)
            , mTo(aTo)
            , mUnbinder()
        {
        }

        virtual void exec()
        {
            ObjectNode* topNode = mTree.topNode();
            if (!topNode) return;

            ObjectNode* targetNode = util::TreeUtil::find(*topNode, mFrom);
            XC_PTR_ASSERT(targetNode);

            mUnbinder.initNode(*targetNode);

            for (ObjectNode* node = targetNode->parent(); node; node = node->parent())
            {
                if (!node->timeLine()) continue;

                auto currentPos = util::TreeUtil::getTreePos(node);
                if (currentPos != mTo && currentPos.contains(mTo)) break;

                auto& map = node->timeLine()->map(TimeKeyType_Bone);

                for (auto itr = map.begin(); itr != map.end(); ++itr)
                {
                    TimeKey* key = itr.value();
                    XC_PTR_ASSERT(key);
                    XC_ASSERT(key->type() == TimeKeyType_Bone);
                    mUnbinder.addAll(*((BoneKey*)key));
                }
            }
            redo();
        }

        virtual void redo()
        {
            mUnbinder.exec();
        }

        virtual void undo()
        {
            mUnbinder.undo();
        }
    };

    return new UnbinderForMove(aTree, aFrom, aTo);
}

} // namespace core
