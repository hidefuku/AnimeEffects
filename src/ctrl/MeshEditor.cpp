#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/MeshEditor.h"
#include "ctrl/mesh/mesh_CreateMode.h"
#include "ctrl/mesh/mesh_DeleteMode.h"
#include "ctrl/mesh/mesh_SplitMode.h"

using namespace core;

namespace ctrl
{

MeshEditor::MeshEditor(Project& aProject)
    : mProject(aProject)
    , mParam()
    , mCurrent()
    , mTarget()
    , mKeyOwner()
{
}

MeshEditor::~MeshEditor()
{
    finalize();
}

void MeshEditor::setTarget(core::ObjectNode* aTarget)
{
    auto prev = mTarget.node;

    finalize();

    if (!aTarget || !aTarget->timeLine()) return;

    // for layer only
    if (aTarget->type() != ObjectType_Layer) return;

    resetTarget(prev, aTarget);
}

void MeshEditor::updateParam(const MeshParam& aParam)
{
    const MeshParam prev = mParam;
    mParam = aParam;

    if (prev.mode != mParam.mode)
    {
        resetCurrentTarget();
    }

    if (mCurrent)
    {
        mCurrent->updateParam(mParam);
    }
}

bool MeshEditor::updateCursor(const core::CameraInfo& aCamera, const core::AbstractCursor& aCursor)
{
    if (mCurrent)
    {
        return mCurrent->updateCursor(aCamera, aCursor);
    }
    return false;
}

void MeshEditor::updateEvent(EventType)
{
    resetCurrentTarget();
}

void MeshEditor::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    if (mCurrent)
    {
        return mCurrent->renderQt(aInfo, aPainter);
    }
}

void MeshEditor::finalize()
{
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();
    mTarget.clear();
}

void MeshEditor::resetTarget(ObjectNode* aPrev, ObjectNode* aNext)
{
    (void)aPrev;
    mTarget.node = aNext;
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();

    // read srt matrix
    if (mTarget)
    {
        XC_PTR_ASSERT(mTarget->timeLine());

        bool success = false;
        mTarget.mtx = mTarget->timeLine()->current().srt().worldMatrix();
        mTarget.mtx.translate(mTarget->timeLine()->current().imageOffset());

        mTarget.invMtx = mTarget.mtx.inverted(&success);
        if (!success)
        {
            mTarget.node = nullptr;
        }
    }

    if (mTarget)
    {
        initializeKey(*mTarget->timeLine());
        createMode();
    }
}

void MeshEditor::resetCurrentTarget()
{
    resetTarget(mTarget.node, mTarget.node);
}

void MeshEditor::createMode()
{
    mCurrent.reset();

    if (!mTarget || !mKeyOwner.key) return;

    switch (mParam.mode)
    {
    case 0:
        mCurrent.reset(new mesh::CreateMode(mProject, mTarget, mKeyOwner));
        break;
    case 1:
        mCurrent.reset(new mesh::DeleteMode(mProject, mTarget, mKeyOwner));
        break;
    case 2:
        mCurrent.reset(new mesh::SplitMode(mProject, mTarget, mKeyOwner));
        break;

    default:
        break;
    }

    if (mCurrent)
    {
        mCurrent->updateParam(mParam);
    }
}

void MeshEditor::initializeKey(TimeLine& aLine)
{
    const TimeLine::MapType& map = aLine.map(TimeKeyType_Mesh);
    const int frame = mProject.animator().currentFrame().get();

    if (map.contains(frame))
    {
        // a key is exists
        mKeyOwner.key = static_cast<MeshKey*>(map.value(frame));
        XC_PTR_ASSERT(mKeyOwner.key);
        mKeyOwner.ownsKey = false;
    }
    else
    {
        // create new key
        mKeyOwner.key = new MeshKey();
        mKeyOwner.ownsKey = true;

        // initialize key data
        if (mTarget.node)
        {
            mKeyOwner.key->data().setImageSize(mTarget.node->initialRect().size());
        }
    }
}

} // namespace ctrl
