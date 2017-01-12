#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/SRTEditor.h"
#include "ctrl/srt/srt_MoveMode.h"
#include "ctrl/srt/srt_CentroidMode.h"

using namespace core;

namespace ctrl
{


//-----------------------------------------------------------------------------------
SRTEditor::SRTEditor(Project& aProject, UILogger& aUILogger)
    : mProject(aProject)
    , mLifeLink()
    , mUILogger(aUILogger)
    , mParam()
    , mTarget()
    , mKeyOwner()
{
}

SRTEditor::~SRTEditor()
{
    finalize();
}

bool SRTEditor::initializeKey(TimeLine& aLine, QString* aMessage)
{
    const TimeLine::MapType& moveMap   = aLine.map(TimeKeyType_Move);
    const TimeLine::MapType& rotateMap = aLine.map(TimeKeyType_Rotate);
    const TimeLine::MapType& scaleMap  = aLine.map(TimeKeyType_Scale);
    TimeKeyExpans& current = aLine.current();
    const int frame = mProject.animator().currentFrame().get();

    // moveKey
    mKeyOwner.ownsMoveKey = !moveMap.contains(frame);
    if (mKeyOwner.ownsMoveKey)
    {
        mKeyOwner.moveKey = new MoveKey();
        mKeyOwner.moveKey->setPos(current.srt().pos());
        mKeyOwner.moveKey->setCentroid(current.srt().centroid());
    }
    else
    {
        mKeyOwner.moveKey = static_cast<MoveKey*>(moveMap.value(frame));
    }

    // rotateKey
    mKeyOwner.ownsRotateKey = !rotateMap.contains(frame);
    if (mKeyOwner.ownsRotateKey)
    {
        mKeyOwner.rotateKey = new RotateKey();
        mKeyOwner.rotateKey->setRotate(current.srt().rotate());
    }
    else
    {
        mKeyOwner.rotateKey = static_cast<RotateKey*>(rotateMap.value(frame));
    }

    // scaleKey
    mKeyOwner.ownsScaleKey = !scaleMap.contains(frame);
    if (mKeyOwner.ownsScaleKey)
    {
        mKeyOwner.scaleKey = new ScaleKey();
        mKeyOwner.scaleKey->setScale(current.srt().scale());
    }
    else
    {
        mKeyOwner.scaleKey = static_cast<ScaleKey*>(scaleMap.value(frame));
    }

    // check validity
    XC_ASSERT(mKeyOwner);

    // setup matrix
    if (!mKeyOwner.updatePosture(current))
    {
        mKeyOwner.deleteOwningKeys();
        if (aMessage)
        {
            *aMessage = UILog::tr("The object which has an invalid posture was given.");
        }
        return false;
    }

    return true;
}

void SRTEditor::finalize()
{
    mCurrent.reset();
    mKeyOwner.deleteOwningKeys();
}

void SRTEditor::createMode()
{
    mCurrent.reset();

    if (!mTarget || !mKeyOwner) return;

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

bool SRTEditor::setTarget(ObjectNode* aTarget)
{
    finalize();

    mTarget = nullptr;

    if (aTarget && aTarget->timeLine())
    {
        mTarget = aTarget;
        QString message;
        if (initializeKey(*mTarget->timeLine(), &message))
        {
            createMode();
        }
        else
        {
            mTarget = nullptr;
            if (!message.isEmpty())
            {
                mUILogger.pushLog(UILog::tr("SRTEditor : ") + message, UILogType_Warn);
            }
        }
    }
    return mTarget && mKeyOwner;
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

