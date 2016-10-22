#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/SRTEditor.h"
#include "ctrl/srt/srt_MoveMode.h"
#include "ctrl/srt/srt_CentroidMode.h"

using namespace core;

namespace ctrl
{


//-----------------------------------------------------------------------------------
SRTEditor::SRTEditor(Project& aProject)
    : mProject(aProject)
    , mLifeLink()
    , mParam()
    , mTarget()
    , mKeyOwner()
{
}

SRTEditor::~SRTEditor()
{
    finalize();
}

bool SRTEditor::initializeKey(TimeLine& aLine)
{
    const TimeLine::MapType& map = aLine.map(TimeKeyType_SRT);
    TimeKeyExpans& current = aLine.current();
    const int frame = mProject.animator().currentFrame().get();

    if (map.contains(frame))
    {
        // a key is exists
        mKeyOwner.key = static_cast<SRTKey*>(map.value(frame));
        XC_PTR_ASSERT(mKeyOwner.key);
        mKeyOwner.ownsKey = false;
    }
    else
    {
        // create new key
        mKeyOwner.key = new SRTKey();
        mKeyOwner.ownsKey = true;
        mKeyOwner.key->data() = current.srt().data();
    }

    if (!mKeyOwner.updatePosture(current))
    {
        mKeyOwner.deleteOwnsKey();
        return false;
    }

    return true;
}

void SRTEditor::finalize()
{
    mCurrent.reset();
    mKeyOwner.deleteOwnsKey();
}

void SRTEditor::createMode()
{
    mCurrent.reset();

    if (!mTarget || !mKeyOwner.key) return;

    switch (mParam.mode)
    {
    case 0:
        mCurrent.reset(new srt::MoveMode(mProject, *mTarget, mKeyOwner));
        break;
    case 1:
        mCurrent.reset(new srt::CentroidMode(mProject, *mTarget, mKeyOwner));
        break;

    default:
        break;
    }

    if (mCurrent)
    {
        mCurrent->updateParam(mParam);
    }
}

void SRTEditor::setTarget(ObjectNode* aTarget)
{
    finalize();

    mTarget = nullptr;

    if (aTarget && aTarget->timeLine())
    {
        mTarget = aTarget;
        if (initializeKey(*mTarget->timeLine()))
        {
            createMode();
        }
        else
        {
            mTarget = nullptr;
        }
    }
}

void SRTEditor::updateParam(const SRTParam& aParam)
{
    const SRTParam prev = mParam;
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

bool SRTEditor::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    if (mCurrent)
    {
        return mCurrent->updateCursor(aCamera, aCursor);
    }
    return false;
}

void SRTEditor::updateEvent(EventType)
{
    resetCurrentTarget();
}

void SRTEditor::resetCurrentTarget()
{
    finalize();

    if (mTarget)
    {
        initializeKey(*mTarget->timeLine());
        createMode();
    }
}

void SRTEditor::renderQt(const RenderInfo& aInfo, QPainter& aPainter)
{
    if (mCurrent)
    {
        return mCurrent->renderQt(aInfo, aPainter);
    }
}

} // namespace core

