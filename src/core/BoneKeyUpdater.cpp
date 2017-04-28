#include "util/TreeUtil.h"
#include "cmnd/Stable.h"
#include "core/BoneKeyUpdater.h"
#include "core/BoneKey.h"
#include "core/Project.h"

namespace core
{

bool keyAffectsToInfluenceMap(TimeKeyType aKeyType)
{
    switch (aKeyType)
    {
    case TimeKeyType_Move:
    case TimeKeyType_Rotate:
    case TimeKeyType_Scale:
    case TimeKeyType_Mesh:
    case TimeKeyType_Image:
        return true;
    default:
        return false;
    }

}

//-------------------------------------------------------------------------------------------------
BoneUnbindWorkspace::BoneUnbindWorkspace()
    : units()
{
}

void BoneUnbindWorkspace::push(ObjectNode& aNode)
{
    units.push_back(Unit());
    auto& unit = units.back();
    unit.node = &aNode;
    for (auto parent = aNode.parent(); parent; parent = parent->parent())
    {
        if (!parent->timeLine() || parent->timeLine()->isEmpty(TimeKeyType_Bone)) continue;
        unit.parents.push_back(parent);
    }
}

BoneUnbindWorkspace::Unit::Unit()
    : parents()
    , node()
{
}

//-------------------------------------------------------------------------------------------------
void BoneKeyUpdater::onTimeLineModified(TimeLineEvent& aEvent)
{
    bool resetCacheList = false;

    // pass only a key which affect to influence map
    QVector<ObjectNode*> targets;
    for (auto t : aEvent.targets())
    {
        if (keyAffectsToInfluenceMap(t.pos.type()))
        {
            targets.push_back(t.node);
        }
    }
    for (auto t : aEvent.defaultTargets())
    {
        if (keyAffectsToInfluenceMap(t.pos.type()))
        {
            targets.push_back(t.node);
        }
    }
    if (aEvent.type() == TimeLineEvent::Type_CopyKey)
    {
        for (auto t : aEvent.targets())
        {
            if (t.pos.type() == TimeKeyType_Bone)
            {
                targets.push_back(t.node);
                resetCacheList = true;
            }
        }
    }
    if (targets.empty()) return;

    // remove redundant nodes
    QVector<ObjectNode*> uniqueRoots = util::TreeUtil::getUniqueRoots(targets);

    for (auto root : uniqueRoots)
    {
        for (auto p = root; p; p = p->parent())
        {
            BoneKeyUpdater::onTimeLineModified(aEvent.project(), *p, uniqueRoots, resetCacheList);
        }
    }
}

void BoneKeyUpdater::onTimeLineModified(
        Project& aProject, ObjectNode& aNode,
        const QVector<ObjectNode*>& aUniqueRoots,
        bool aResetCacheList)
{
    if (!aNode.timeLine()) return;
    auto& map = aNode.timeLine()->map(TimeKeyType_Bone);
    for (auto itr = map.begin(); itr != map.end(); ++itr)
    {
        TimeKey* key = itr.value();
        TIMEKEY_PTR_TYPE_ASSERT(key, Bone);
        if (aResetCacheList)
        {
            ((BoneKey*)key)->resetCaches(aProject, aNode);
        }
        else
        {
            ((BoneKey*)key)->updateCaches(aProject, aNode, aUniqueRoots);
        }
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
            TIMEKEY_PTR_TYPE_ASSERT(key, Bone);
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
        if (!aBone.bindingNodes().contains(mNode)) return false;
        Pos pos = { &aBone, -1 };
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
        for (auto& pos : mPositions)
        {
            if (pos.index < 0)
            {
                pos.index = pos.bone->bindingNodes().indexOf(mNode);
                XC_ASSERT(pos.index >= 0);
            }
            //qDebug() << "exec unbind" << mNode->name() << pos.bone << pos.index;
            pos.bone->bindingNodes().removeAt(pos.index);
        }
    }

    void undo()
    {
        for (auto pos : mPositions)
        {
            XC_ASSERT(pos.index >= 0);
            //qDebug() << "undo unbind" << mNode->name();
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
                    TIMEKEY_PTR_TYPE_ASSERT(key, Bone);
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

#if 0
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
                    TIMEKEY_PTR_TYPE_ASSERT(key, Bone);
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
#endif

cmnd::Base* BoneKeyUpdater::createNodesUnbinderForMove(
        ObjectTree& aTree, const BoneUnbindWorkspacePtr& aWorkspace)
{
    class NodesUnbinderForMove : public cmnd::Stable
    {
        ObjectTree& mTree;
        BoneUnbindWorkspacePtr mWorkspace;
        QList<Unbinder*> mUnbinders;
    public:
        NodesUnbinderForMove(ObjectTree& aTree, const BoneUnbindWorkspacePtr& aWorkspace)
            : mTree(aTree)
            , mWorkspace(aWorkspace)
            , mUnbinders()
        {
        }

        ~NodesUnbinderForMove()
        {
            qDeleteAll(mUnbinders);
        }

        virtual void exec()
        {
            using util::TreeUtil;
            XC_ASSERT(mWorkspace);

            // each moved nodes
            for (auto& unit : mWorkspace->units)
            {
                // all children of the node
                ObjectNode::Iterator nodeItr(unit.node);
                while (nodeItr.hasNext())
                {
                    auto node = nodeItr.next();
                    XC_PTR_ASSERT(node);
                    Unbinder* unbinder = nullptr;
                    // all parent node which has bone
                    for (auto parent : unit.parents)
                    {
                        XC_PTR_ASSERT(parent);
                        XC_PTR_ASSERT(parent->timeLine());
                        if (TreeUtil::leftContainsRight<ObjectNode>(*parent, *node))
                        {
                            continue;
                        }

                        auto& map = parent->timeLine()->map(TimeKeyType_Bone);
                        for (auto itr = map.begin(); itr != map.end(); ++itr)
                        {
                            TimeKey* key = itr.value();
                            TIMEKEY_PTR_TYPE_ASSERT(key, Bone);

                            if (!unbinder)
                            {
                                unbinder = new Unbinder();
                                mUnbinders.push_back(unbinder);
                                unbinder->initNode(*node);
                            }
                            unbinder->addAll(*((BoneKey*)key));
                        }
                    }
                }
            }
            mWorkspace.reset();
            redo();
        }

        virtual void redo()
        {
            for (auto unbinder : mUnbinders)
            {
                unbinder->exec();
            }
        }

        virtual void undo()
        {
            for (auto unbinder : mUnbinders)
            {
                unbinder->undo();
            }
        }
    };

    return new NodesUnbinderForMove(aTree, aWorkspace);
}

} // namespace core
