#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/bone/bone_BindNodesMode.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/bone/bone_Notifier.h"

using namespace core;

namespace ctrl {
namespace bone {

BindNodesMode::BindNodesMode(Project& aProject, const Target& aTarget,
                             KeyOwner& aKey, const GraphicStyle& aGraphicStyle)
    : mProject(aProject)
    , mGraphicStyle(aGraphicStyle)
    , mTarget(*aTarget.node)
    , mKeyOwner(aKey)
    , mTargetMtx(aTarget.mtx)
    , mTargetInvMtx(aTarget.invMtx)
    , mFocuser()
    , mNodeSelector(*aTarget.node, aGraphicStyle)
{
    XC_PTR_ASSERT(mKeyOwner.key);
    mFocuser.setTopBones(mKeyOwner.key->data().topBones());
    mFocuser.setTargetMatrix(mTargetMtx);
    mFocuser.setFocusConnector(true);

    mNodeSelector.initGeometries();
}

bool BindNodesMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto boneFocus = mFocuser.update(aCamera, aCursor.screenPos());
    mNodeSelector.sortCurrentGeometries(aCamera);
    mNodeSelector.updateFocus(aCamera, aCursor.screenPos());

    bool updated = mFocuser.focusChanged() || mNodeSelector.focusChanged();

    if (!mKeyOwner.owns())
    {
        if (aCursor.isLeftPressState())
        {
            mFocuser.clearFocus();
            auto newSelectNode = mNodeSelector.click(aCamera);

            if (newSelectNode)
            {
                unbindNode(*newSelectNode);
            }
            else if (boneFocus)
            {
                mFocuser.clearSelection();
                mFocuser.select(*boneFocus);
            }

            auto selectNode = mNodeSelector.selectingNode();
            auto selectBone = mFocuser.selectingBone();
            if (selectNode && selectBone)
            {
                bindNode(*selectBone, *selectNode);
                mFocuser.clearSelection();
                mNodeSelector.clearSelection();
            }
            updated = true;
        }
        else if (aCursor.isLeftMoveState())
        {
        }
        else if (aCursor.isLeftReleaseState())
        {
            updated = true;
        }
    }

    return updated;
}

void BindNodesMode::bindNode(Bone2& aBone, ObjectNode& aNode)
{
    XC_ASSERT(!mKeyOwner.owns());
    if (aBone.isBinding(aNode)) return; // the node was already bound

    cmnd::Stack& stack = mProject.commandStack();
    {
        cmnd::ScopedMacro macro(stack, "bind a node to a bone");
        macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key,
                                        TimeLineEvent::Type_ChangeKeyValue));

        stack.push(new cmnd::PushBackList<ObjectNode*>(&aBone.bindingNodes(), &aNode));
    }
}

void BindNodesMode::unbindNode(ObjectNode& aNode)
{
    XC_ASSERT(!mKeyOwner.owns());

    typedef std::pair<Bone2*, ObjectNode*> BindPair;
    QList<BindPair> pairs;
    for (auto topBone : mKeyOwner.key->data().topBones())
    {
        Bone2::Iterator itr(topBone);
        while (itr.hasNext())
        {
            auto bone = itr.next();
            if (bone->isBinding(aNode))
            {
                pairs.push_back(BindPair(bone, &aNode));
            }
        }
    }

    cmnd::Stack& stack = mProject.commandStack();
    cmnd::ScopedMacro macro(stack, "unbind a node from bones");
    macro.grabListener(new Notifier(mProject, mTarget, *mKeyOwner.key,
                                    TimeLineEvent::Type_ChangeKeyValue));

    for (auto pair : pairs)
    {
        stack.push(new cmnd::RemoveListByObj<ObjectNode*>(&(pair.first->bindingNodes()), pair.second));
    }
}

void BindNodesMode::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    bone::Renderer renderer(aPainter, aInfo);
    renderer.setAntialiasing(true);
    renderer.setTargetMatrix(mTargetMtx);
    renderer.setFocusConnector(true);

    mNodeSelector.sortCurrentGeometries(aInfo.camera);

    if (!mKeyOwner.owns())
    {
        for (auto topBone : mKeyOwner.key->data().topBones())
        {
            mNodeSelector.renderBindings(aInfo, aPainter, mTargetMtx, topBone);
        }

        for (auto topBone : mKeyOwner.key->data().topBones())
        {
            renderer.renderBones(topBone);
        }
    }

    mNodeSelector.renderTags(aInfo, aPainter);
}

} // namespace bone
} // namespace ctrl
