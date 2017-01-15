#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/MeshEditor.h"
#include "ctrl/mesh/mesh_CreateMode.h"
#include "ctrl/mesh/mesh_DeleteMode.h"
#include "ctrl/mesh/mesh_SplitMode.h"

using namespace core;

namespace ctrl
{

MeshEditor::MeshEditor(Project& aProject, UILogger& aUILogger)
    : mProject(aProject)
    , mUILogger(aUILogger)
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

bool MeshEditor::setTarget(core::ObjectNode* aTarget)
{
    auto prev = mTarget.node;

    finalize();

    if (!aTarget || !aTarget->timeLine()) return false;

    auto group = UILog::tr("MeshEditor : ");

    // for layer only
    if (aTarget->type() != ObjectType_Layer)
    {
        mUILogger.pushLog(group + UILog::tr("The object can't own a mesh key."), UILogType_Info);
        return false;
    }

    QString message;

    resetTarget(prev, aTarget, &message);

    if (!message.isEmpty())
    {
        mUILogger.pushLog(group + message, UILogType_Warn);
    }

    return mTarget && mKeyOwner;
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

void MeshEditor::resetTarget(ObjectNode* aPrev, ObjectNode* aNext, QString* aMessage)
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
        mTarget.mtx = mTarget->timeLine()->current().srt().worldCSRTMatrix();
        //mTarget.mtx.translate(mTarget->timeLine()->current().imageOffset());

        mTarget.invMtx = mTarget.mtx.inverted(&success);
        if (!success)
        {
            mTarget.node = nullptr;
            if (aMessage)
            {
                *aMessage = UILog::tr("The object which has an invalid posture was given.");
            }
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
    }
}

} // namespace ctrl
