#ifndef CORE_OBJECTTREE_H
#define CORE_OBJECTTREE_H

#include <tuple>
#include <QVector>
#include "util/TreePos.h"
#include "util/LifeLink.h"
#include "util/NonCopyable.h"
#include "cmnd/Stack.h"
#include "cmnd/Vector.h"
#include "core/ObjectNode.h"
#include "core/Renderer.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"
#include "core/ResourceEvent.h"
#include "core/TimeLineEvent.h"
#include "core/ProjectEvent.h"
#include "core/ShaderHolder.h"
#include "core/TimeCacheLock.h"
namespace core { class SortAndRenderCall; }

namespace core
{

class ObjectTree
        : private util::NonCopyable
{
public:
    ObjectTree();
    ~ObjectTree();

    util::LifeLink::Pointee<ObjectTree> pointee() { return mLifeLink.pointee<ObjectTree>(this); }
    util::LifeLink::Pointee<const ObjectTree> constPointee() { return mLifeLink.pointee<const ObjectTree>(this); }

    void grabTopNode(ObjectNode* aNode) { mTopNode.reset(aNode); }
    ObjectNode* topNode() { return mTopNode.data(); }
    const ObjectNode* topNode() const { return mTopNode.data(); }

    ShaderHolder& shaderHolder() { return mShaderHolder; }
    const ShaderHolder& shaderHolder() const { return mShaderHolder; }

    TimeCacheLock& timeCacheLock() { return mTimeCacheLock; }
    const TimeCacheLock& timeCacheLock() const { return mTimeCacheLock; }

    void render(const RenderInfo& aRenderInfo, bool aUseWorkingCache);

    cmnd::Vector createNodeDeleter(ObjectNode& aNode);
    cmnd::Vector createNodeMover(const util::TreePos& aFrom, const util::TreePos& aTo);
    cmnd::Vector createResourceUpdater(const ResourceEvent& aEvent);

    void onTimeLineModified(TimeLineEvent& aEvent, bool aIsUndo);
    void onTreeRestructured(ObjectTreeEvent& aEvent, bool aIsUndo);
    void onResourceModified(ResourceEvent& aEvent, bool aIsUndo);
    void onProjectAttributeModified(ProjectEvent& aEvent, bool aIsUndo);

    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    ObjectNode* findNode(const util::TreePos& aPos);
    ObjectNode* eraseNode(const util::TreePos& aPos);
    void insertNode(const util::TreePos& aPos, ObjectNode* aNode);
    bool serializeNode(Serializer& aOut, const ObjectNode& aNode) const;
    bool deserializeNode(Deserializer& aIn, ObjectNode* aParent);
    ObjectNode* createSerialNode(int aType);

    util::LifeLink mLifeLink;
    QScopedPointer<ObjectNode> mTopNode;
    QScopedPointer<SortAndRenderCall> mCaller;
    ShaderHolder mShaderHolder;
    TimeCacheLock mTimeCacheLock;
};

} // namespace core

#endif // CORE_OBJECTTREE_H
