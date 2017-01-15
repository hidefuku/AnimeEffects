#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/PoseEditor.h"
#include "ctrl/bone/bone_Renderer.h"
#include "ctrl/pose/pose_TransBoneMode.h"

using namespace core;

namespace ctrl
{

PoseEditor::PoseEditor(Project& aProject, UILogger& aUILogger)
    : mProject(aProject)
    , mUILogger(aUILogger)
    , mTarget()
    , mKeyOwner()
    , mCurrent()
{
}

PoseEditor::~PoseEditor()
{
    finalize();
}

bool PoseEditor::setTarget(core::ObjectNode* aTarget)
{
    finalize();

    if (!aTarget || !aTarget->timeLine()) return false;

    mTarget.node = aTarget;
    QString message;
    resetCurrentTarget(&message);

    if (!message.isEmpty())
    {
        mUILogger.pushLog(UILog::tr("PoseEditor : ") + message, UILogType_Warn);
    }

    return mTarget && mKeyOwner;
}

#if 0
void PoseEditor::updateParam(const PoseParam& aParam)
{
    const PoseParam prev = mParam;
    mParam = aParam;

    if (prev.mode != mParam.mode)
    {
        resetCurrentTarget();
    }
}
#endif

bool PoseEditor::updateCursor(const core::CameraInfo& aCamera, const core::AbstractCursor& aCursor)
{
    if (mCurrent)
    {
        return mCurrent->updateCursor(aCamera, aCursor);
    }
    return false;
}

void PoseEditor::updateEvent(EventType)
{
    resetCurrentTarget();
}

void PoseEditor::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    if (mTarget && mKeyOwner.key && mCurrent)
    {
        mCurrent->renderQt(aInfo, aPainter);
    }
}

void PoseEditor::finalize()
{
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();
    mTarget.clear();
}

void PoseEditor::resetCurrentTarget(QString* aMessage)
{
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();

    // read srt matrix
    if (mTarget)
    {
        XC_PTR_ASSERT(mTarget->timeLine());
        auto& current = mTarget->timeLine()->current();

        bool success = false;

        if (current.bone().isAffectedByBinding())
        {
            mTarget.mtx = current.bone().worldCSRTMatrix();
        }
        else
        {
            mTarget.mtx = current.srt().worldCSRTMatrix();
        }

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
        if (initializeKey(*mTarget->timeLine()))
        {
            mCurrent.reset(new pose::TransBoneMode(mProject, mTarget, mKeyOwner));
        }
        else
        {
            if (aMessage)
            {
                *aMessage = UILog::tr("There is no bone key which can be a parent.");
            }
        }
    }
}

bool PoseEditor::initializeKey(TimeLine& aLine)
{
    const TimeLine::MapType& map = aLine.map(TimeKeyType_Pose);
    TimeKeyExpans& current = aLine.current();
    const int frame = mProject.animator().currentFrame().get();

    if (map.contains(frame))
    {
        // a key is exists
        mKeyOwner.key = static_cast<PoseKey*>(map.value(frame));
        XC_PTR_ASSERT(mKeyOwner.key);
        mKeyOwner.ownsKey = false;
        mKeyOwner.parent = mKeyOwner.key->parent();
        return true;
    }
    else if (current.bone().areaKey())
    {
        // create new key
        mKeyOwner.key = new PoseKey();
        mKeyOwner.ownsKey = true;
        mKeyOwner.parent = current.bone().areaKey();

        if (current.bone().areaKey() == current.poseParent())
        {
            // copy blended pose
            mKeyOwner.key->data() = current.pose();
        }
        else
        {
            // create from original bones
            mKeyOwner.key->data().createBonesBy(*current.bone().areaKey());
        }
        return true;
    }

    return false;
}

} // namespace ctrl
