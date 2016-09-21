#include <algorithm>
#include <functional>
#include "util/LinkPointer.h"
#include "util/TreeUtil.h"
#include "cmnd/Stable.h"
#include "cmnd/Vector.h"
#include "cmnd/BasicCommands.h"
#include "core/ObjectTree.h"
#include "core/LayerNode.h"
#include "core/LayerSetNode.h"
#include "core/ObjectTreeEvent.h"
#include "core/TimeCacheAccessor.h"
#include "core/BoneKeyUpdater.h"

namespace core
{

//-------------------------------------------------------------------------------------------------
class SortAndRenderCall
{
    static bool compareDepth(Renderer* a, Renderer* b)
    {
        return a->renderDepth() < b->renderDepth();
    }

    void pushNodeRecursive(ObjectNode* aNode, float aParentDepth, bool aPush)
    {
        if (!aNode || !aNode->isVisible()) return;

        const float depth = aParentDepth + aNode->depth();
        auto renderer = aNode->renderer();

        if (renderer)
        {
            renderer->setRenderDepth(depth);

            if (aPush)
            {
                if (!renderer->isClipped())
                {
                    mArray.push_back(renderer);
                }
                else
                {
                    aPush = false;
                }

            }
        }

        auto& children = aNode->children();
        for (auto itr = children.rbegin(); itr != children.rend(); ++itr)
        {
            pushNodeRecursive(*itr, depth, aPush);
        }
    }

    std::vector<Renderer*> mArray;

public:

    SortAndRenderCall()
        : mArray()
    {
    }

    void invoke(ObjectNode* aTopNode,
                const RenderInfo& aInfo,
                const TimeCacheAccessor& aAccessor)
    {
        // prerender
        {
            ObjectNode::Iterator itr(aTopNode);
            while (itr.hasNext())
            {
                auto renderer = itr.next()->renderer();
                if (renderer)
                {
                    renderer->prerender(aInfo, aAccessor);
                }
            }
        }

        // sort
        mArray.clear();
        pushNodeRecursive(aTopNode, 0.0f, true);
        std::stable_sort(mArray.begin(), mArray.end(), compareDepth);

        // render
        for (auto data : mArray)
        {
            data->render(aInfo, aAccessor);
        }
    }
};

}

namespace core
{

//-------------------------------------------------------------------------------------------------
ObjectTree::ObjectTree()
    : mLifeLink()
    , mTopNode()
    , mCaller(new SortAndRenderCall())
    , mShaderHolder()
    , mTimeCacheLock()
{
}

ObjectTree::~ObjectTree()
{
}

void ObjectTree::render(const RenderInfo& aInfo, bool aUseWorkingCache)
{
    if (mTopNode.data())
    {
        TimeCacheAccessor accessor(
                    *mTopNode.data(), mTimeCacheLock, aInfo.time, aUseWorkingCache);
        mCaller->invoke(mTopNode.data(), aInfo, accessor);
    }
}

cmnd::Vector ObjectTree::createNodeDeleter(ObjectNode& aNode)
{
    cmnd::Vector commands;

    core::ObjectNode* parent = aNode.parent();
    XC_PTR_ASSERT(parent);
    if (!parent) return commands; // fail safe code

    auto index = parent->children().indexOf(&aNode);
    XC_ASSERT(index >= 0);
    if (index < 0) return commands; // fail safe code

    commands.push(BoneKeyUpdater::createNodeUnbinderForDelete(aNode));
    commands.push(new cmnd::RemoveTree<core::ObjectNode>(&(parent->children()), index));
    commands.push(new cmnd::GrabDeleteObject<core::ObjectNode>(&aNode));

    return commands;
}

cmnd::Vector ObjectTree::createNodeMover(
        const util::TreePos& aFrom, const util::TreePos& aTo)
{
    class MoveNodeCommand : public cmnd::Stable
    {
        util::LinkPointer<ObjectTree> mTree;
        util::TreePos mFrom;
        util::TreePos mTo;

    public:
        MoveNodeCommand(ObjectTree& aTree, const util::TreePos& aFrom, const util::TreePos& aTo)
            : mTree(aTree.pointee())
            , mFrom(aFrom)
            , mTo(aTo)
        {
        }

        virtual void undo()
        {
            if (!mTree) return;
            ObjectNode* target = mTree->eraseNode(mTo);
            mTree->insertNode(mFrom, target);
        }

        virtual void redo()
        {
            if (!mTree) return;
            ObjectNode* target = mTree->eraseNode(mFrom);
            mTree->insertNode(mTo, target);
        }
    };

    cmnd::Vector commands;
    commands.push(BoneKeyUpdater::createNodeUnbinderForMove(*this, aFrom, aTo));
    commands.push(new MoveNodeCommand(*this, aFrom, aTo));
    return commands;
}

ObjectNode* ObjectTree::findNode(const util::TreePos& aPos)
{
    if (!aPos.isValid()) return nullptr;

    ObjectNode::Children::Iterator itr;
    ObjectNode* current = topNode();

    for (int i = 1; i < aPos.depth() - 1; ++i)
    {
        itr = current->children().at(aPos.row(i));
        XC_ASSERT(itr != current->children().end());
        current = *itr;
    }

    itr = current->children().at(aPos.tailRow());
    XC_ASSERT(itr != current->children().end());

    ObjectNode* target = *itr;
    return target;
}

ObjectNode* ObjectTree::eraseNode(const util::TreePos& aPos)
{
    ObjectNode::Children::Iterator itr;
    ObjectNode* current = topNode();

    for (int i = 1; i < aPos.depth() - 1; ++i)
    {
        itr = current->children().at(aPos.row(i));
        current = *itr;
    }

    itr = current->children().at(aPos.tailRow());

    ObjectNode* target = *itr;
    current->children().erase(itr);
    return target;
}

void ObjectTree::insertNode(const util::TreePos& aPos, ObjectNode* aNode)
{
    XC_PTR_ASSERT(aNode);
    ObjectNode::Children::Iterator itr;
    ObjectNode* current = topNode();

    for (int i = 1; i < aPos.depth() - 1; ++i)
    {
        itr = current->children().at(aPos.row(i));
        current = *itr;
    }

    itr = current->children().at(aPos.tailRow());
    current->children().insert(itr, aNode);
}

cmnd::Vector ObjectTree::createResourceUpdater(const ResourceEvent& aEvent)
{
    cmnd::Vector result;
    if (mTopNode)
    {
        ObjectNode::Iterator itr(mTopNode.data());
        while (itr.hasNext())
        {
            result.push(itr.next()->createResourceUpdater(aEvent));
        }
    }
    return result;
}

void ObjectTree::onTimeLineModified(TimeLineEvent& aEvent, bool)
{
    BoneKeyUpdater::onTimeLineModified(aEvent);
}

void ObjectTree::onTreeRestructured(ObjectTreeEvent& aEvent, bool)
{
    BoneKeyUpdater::onTreeRestructured(aEvent);
}

void ObjectTree::onResourceModified(ResourceEvent& aEvent, bool)
{
    BoneKeyUpdater::onResourceModified(aEvent);
}

void ObjectTree::onProjectAttributeModified(ProjectEvent& aEvent, bool)
{
    BoneKeyUpdater::onProjectAttributeModified(aEvent);
}

bool ObjectTree::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'O', 'b', 'j', 'T', 'r', 'e', 'e', '_' };

    // signature
    auto pos = aOut.beginBlock(kSignature);

    // top node count
    aOut.write(topNode() ? 1 : 0);

    // nodes
    if (topNode())
    {
        if (!serializeNode(aOut, *topNode()))
        {
            return false;
        }
    }

    aOut.endBlock(pos);

    return aOut.checkStream();
}

bool ObjectTree::serializeNode(Serializer& aOut, const ObjectNode& aNode) const
{
    static const std::array<uint8, 8> kSignature =
        { 'O', 'b', 'j', 'N', 'o', 'd', 'e', '_' };

    // block begin
    auto pos = aOut.beginBlock(kSignature);

    // type
    aOut.write((int)aNode.type());

    // child count
    aOut.write((int)aNode.children().size());

    // reference id
    aOut.writeID(&aNode);

    // serialize object node
    if (!aNode.serialize(aOut))
    {
        return false;
    }

    // block end
    aOut.endBlock(pos);

    // check failure
    if (aOut.failure())
    {
        return false;
    }

    // iterate children
    if (aNode.canHoldChild())
    {
        for (auto child : aNode.children())
        {
            XC_PTR_ASSERT(child);

            if (!serializeNode(aOut, *child))
            {
                return false;
            }
        }
    }

    return !aOut.failure();
}

bool ObjectTree::deserialize(Deserializer& aIn)
{
    // clear
    mTopNode.reset();

    // check block begin
    if (!aIn.beginBlock("ObjTree_"))
        return aIn.errored("invalid signature of object tree");

    // dive log scope
    aIn.pushLogScope("ObjectTree");

    // top node count
    int topNodeCount = 0;
    aIn.read(topNodeCount);

    if (topNodeCount == 1)
    {
        aIn.pushLogScope("ObjectNodes");

        // deserialize each node
        if (!deserializeNode(aIn, nullptr)) return false;

        aIn.popLogScope();
    }
    else if (topNodeCount != 0)
    {
        return aIn.errored("invalid top node count");
    }

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of object tree");

    // rise log scope
    aIn.popLogScope();

    // end
    return aIn.checkStream();
}

bool ObjectTree::deserializeNode(Deserializer &aIn, ObjectNode* aParent)
{
    // check block begin
    if (!aIn.beginBlock("ObjNode_"))
        return aIn.errored("invalid signature of object node");

    // dive log scope
    if (aParent)
        aIn.pushLogScope(aParent->name());

    // node type
    int type = 0;
    aIn.read(type);
    if (type >= ObjectType_TERM)
        return aIn.errored("invalid node type");

    // child count
    int childCount = 0;
    aIn.read(childCount);
    if (childCount < 0)
        return aIn.errored("invalid child count");

    // create node
    ObjectNode* node(createSerialNode(type));
    if (!node)
        return aIn.errored("failed to create node");

    // reference id
    if (!aIn.bindIDData(node))
    {
        delete node;
        return aIn.errored("failed to bind reference id");
    }

    // deserialize node
    if (!node->deserialize(aIn))
    {
        delete node;
        return aIn.errored("failed to deserialize node");
    }

    // push to tree
    if (!aParent)
    {
        grabTopNode(node);
    }
    else
    {
        aParent->children().pushBack(node);
    }

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of object node");

    // check failure
    if (aIn.failure())
        return aIn.errored("stream error");

    // check holdability
    if (!node->canHoldChild() && childCount > 0)
        return aIn.errored("invalid holdability");

    // iterate children
    for (int i = 0; i < childCount; ++i)
    {
        if (!deserializeNode(aIn, node)) return false;
    }

    // rise log scope
    if (aParent)
        aIn.popLogScope();

    // progress report
    aIn.reportCurrent();

    // end
    return aIn.checkStream();
}

ObjectNode* ObjectTree::createSerialNode(int aType)
{
    ObjectNode* node = nullptr;

    if (aType == ObjectType_Layer)
    {
        LayerNode* layer = new LayerNode(QString("no name"), mShaderHolder);
        layer->setVisibility(true);
        node = layer;
    }
    else if (aType == ObjectType_LayerSet)
    {
        LayerSetNode* layerSet = new LayerSetNode(QString("no name"));
        layerSet->setVisibility(true);
        node = layerSet;
    }

    return node;
}

} // namespace core
