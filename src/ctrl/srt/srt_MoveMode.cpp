#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/CmndName.h"
#include "ctrl/srt/srt_MoveMode.h"

using namespace core;

namespace ctrl {
namespace srt {

MoveMode::MoveMode(Project& aProject, ObjectNode& aTarget, KeyOwner& aKeyOwner)
    : mProject(aProject)
    , mTarget(aTarget)
    , mKeyOwner(aKeyOwner)
    , mSRTParam()
    , mSymbol()
    , mFocus(FocusType_TERM, QVector2D())
    , mAssignMoveRef()
    , mAssignRotateRef()
    , mAssignScaleRef()
    , mSuspend()
    , mBaseVec()
    , mBaseValue()
{
    XC_PTR_ASSERT(mTarget.timeLine());
}

void MoveMode::updateParam(const SRTParam& aParam)
{
    mSRTParam = aParam;
}

bool MoveMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto keyLocalMtx = mKeyOwner.getLocalSRTMatrixFromKeys();
    auto& keyParentMtx = mKeyOwner.parentMtx;

    bool mod = false;

    // update symbol
    mSymbol.build(keyLocalMtx, keyParentMtx, aCamera);

    if (aCursor.emitsLeftPressedEvent())
    {
        mFocus = mSymbol.findFocus(aCursor.screenPos());
        auto focusVector = aCamera.toWorldVector(mFocus.second);
        if (mFocus.first != FocusType_TERM)
        {
            mSuspend.construct(mProject);
            mAssignMoveRef = nullptr;
            mAssignRotateRef = nullptr;
            mAssignScaleRef = nullptr;

            if (mFocus.first == FocusType_Scale ||
                mFocus.first == FocusType_ScaleX ||
                mFocus.first == FocusType_ScaleY)
            {
                auto keyWorldPos = keyParentMtx * QVector3D(mKeyOwner.moveKey->pos());
                mBaseVec = mKeyOwner.scaleKey->scale();

                const QVector3D pos = QVector3D(aCursor.worldPos());
                const QVector2D vec = (pos - keyWorldPos).toVector2D();
                const float length = QVector2D::dotProduct(vec, focusVector);
                mBaseValue = std::max(Constant::normalizable(), length);
            }
            else if (mFocus.first == srt::FocusType_Rotate)
            {
                mBaseVec = (mKeyOwner.invParentSRMtx * QVector3D(focusVector)).toVector2D();
            }
            mod = true;
        }
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        auto focusVector = aCamera.toWorldVector(mFocus.second);

        auto keyPos = mKeyOwner.moveKey->pos();
        auto keyWorldPos = keyParentMtx * QVector3D(keyPos);
        auto cursorWorldPos = QVector3D(aCursor.worldPos());
        auto cursorWorldVel = QVector3D(aCursor.worldVel());

        if (mFocus.first == srt::FocusType_Trans)
        {
            auto moveData = mKeyOwner.moveKey->data();
            moveData.addPos((mKeyOwner.invParentSRMtx * cursorWorldVel).toVector2D());
            assignMoveKey(moveData);
        }
        else if (mFocus.first == srt::FocusType_Rotate)
        {
            auto rotData = mKeyOwner.rotateKey->data();
            auto vec = (mKeyOwner.invParentMtx * cursorWorldPos).toVector2D() - keyPos;
            const float length = vec.length();
            if (length > 1.0f)
            {
                auto rotate = util::MathUtil::getAngleDifferenceRad(mBaseVec, vec);
                rotData.addRotate(rotate);
                mBaseVec = vec.normalized();
            }
            assignRotateKey(rotData);
        }
        else if (mFocus.first == srt::FocusType_Scale)
        {
            auto scaleData = mKeyOwner.scaleKey->data();
            const QVector2D vec = (cursorWorldPos - keyWorldPos).toVector2D();
            const float length = QVector2D::dotProduct(vec, focusVector);
            scaleData.setScaleX(mBaseVec.x() * (length / mBaseValue));
            scaleData.setScaleY(mBaseVec.y() * (length / mBaseValue));
            assignScaleKey(scaleData);
        }
        else if (mFocus.first == srt::FocusType_ScaleX)
        {
            auto scaleData = mKeyOwner.scaleKey->data();
            const QVector2D vec = (cursorWorldPos - keyWorldPos).toVector2D();
            const float length = QVector2D::dotProduct(vec, focusVector);
            scaleData.setScaleX(mBaseVec.x() * (length / mBaseValue));
            assignScaleKey(scaleData);
        }
        else if (mFocus.first == srt::FocusType_ScaleY)
        {
            auto scaleData = mKeyOwner.scaleKey->data();
            const QVector2D vec = (cursorWorldPos - keyWorldPos).toVector2D();
            const float length = QVector2D::dotProduct(vec, focusVector);
            scaleData.setScaleY(mBaseVec.y() * (length / mBaseValue));
            assignScaleKey(scaleData);
        }

        if (mFocus.first != srt::FocusType_TERM)
        {
            mod = true;
        }
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        clearState();
        mod = true;
    }
    else
    {
        srt::FocusType prev = mFocus.first;
        mFocus = mSymbol.findFocus(aCursor.screenPos());
        mod = mFocus.first != prev;
    }

    return mod;
}

void MoveMode::renderQt(const core::RenderInfo& aInfo, QPainter& aPainter)
{
    mSymbol.build(mKeyOwner.getLocalSRTMatrixFromKeys(), mKeyOwner.parentMtx, aInfo.camera);
    mSymbol.draw(aInfo, aPainter, mFocus.first);
}

void MoveMode::clearState()
{
    mAssignMoveRef = nullptr;
    mAssignRotateRef = nullptr;
    mAssignScaleRef = nullptr;
    mFocus.first = FocusType_TERM;
    mSuspend.destruct();
}

void MoveMode::setAssignNotifier(
        cmnd::ScopedMacro& aMacro, TimeKeyType aKeyType, bool aOwnsKey)
{
    auto notifyType = aOwnsKey ?
                TimeLineEvent::Type_PushKey :
                TimeLineEvent::Type_ChangeKeyValue;
    const int frame = mProject.animator().currentFrame().get();

    auto notifier = new TimeLineUtil::Notifier(mProject);
    notifier->event().setType(notifyType);
    notifier->event().pushTarget(mTarget, aKeyType, frame);
    aMacro.grabListener(notifier);
}

void MoveMode::notifyAssignModification(TimeKeyType aKeyType)
{
    const int frame = mProject.animator().currentFrame().get();

    // singleshot notify
    TimeLineEvent event;
    event.setType(TimeLineEvent::Type_ChangeKeyValue);
    event.pushTarget(mTarget, aKeyType, frame);
    mProject.onTimeLineModified(event, false);
}

void MoveMode::assignMoveKey(MoveKey::Data& aNewData)
{
    const int frame = mProject.animator().currentFrame().get();
    TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();

    const bool appendKeys =
            (mSRTParam.necessarilyRotate && mKeyOwner.ownsRotateKey) ||
            (mSRTParam.necessarilyScale && mKeyOwner.ownsScaleKey);

    if (!appendKeys && mAssignMoveRef && mProject.commandStack().isModifiable(mAssignMoveRef))
    {
        // modify command
        XC_ASSERT(!mKeyOwner.ownsMoveKey);
        mAssignMoveRef->modifyValue(aNewData);
        notifyAssignModification(TimeKeyType_Move);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("update a moving key"));
        setAssignNotifier(macro, TimeKeyType_Move, mKeyOwner.ownsMoveKey);
        // push commands
        mKeyOwner.pushOwningMoveKey(stack, timeLine, frame);

        if (mSRTParam.necessarilyRotate)
        {
            mKeyOwner.pushOwningRotateKey(stack, timeLine, frame);
        }
        if (mSRTParam.necessarilyScale)
        {
            mKeyOwner.pushOwningScaleKey(stack, timeLine, frame);
        }

        mAssignMoveRef = new AssignMoveCommand(&(mKeyOwner.moveKey->data()), aNewData);
        stack.push(mAssignMoveRef);
    }
}

void MoveMode::assignRotateKey(RotateKey::Data& aNewData)
{
    const int frame = mProject.animator().currentFrame().get();
    TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();

    const bool appendKeys =
            (mSRTParam.necessarilyMove && mKeyOwner.ownsMoveKey) ||
            (mSRTParam.necessarilyScale && mKeyOwner.ownsScaleKey);

    if (!appendKeys && mAssignRotateRef && mProject.commandStack().isModifiable(mAssignRotateRef))
    {
        // modify command
        XC_ASSERT(!mKeyOwner.ownsRotateKey);
        mAssignRotateRef->modifyValue(aNewData);
        notifyAssignModification(TimeKeyType_Rotate);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("update a rotation key"));
        setAssignNotifier(macro, TimeKeyType_Rotate, mKeyOwner.ownsRotateKey);
        // push commands
        mKeyOwner.pushOwningRotateKey(stack, timeLine, frame);

        if (mSRTParam.necessarilyMove)
        {
            mKeyOwner.pushOwningMoveKey(stack, timeLine, frame);
        }
        if (mSRTParam.necessarilyScale)
        {
            mKeyOwner.pushOwningScaleKey(stack, timeLine, frame);
        }

        mAssignRotateRef = new AssignRotateCommand(&(mKeyOwner.rotateKey->data()), aNewData);
        stack.push(mAssignRotateRef);
    }
}

void MoveMode::assignScaleKey(ScaleKey::Data& aNewData)
{
    const int frame = mProject.animator().currentFrame().get();
    TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();

    const bool appendKeys =
            (mSRTParam.necessarilyMove && mKeyOwner.ownsMoveKey) ||
            (mSRTParam.necessarilyRotate && mKeyOwner.ownsRotateKey);

    if (!appendKeys && mAssignScaleRef && mProject.commandStack().isModifiable(mAssignScaleRef))
    {
        // modify command
        XC_ASSERT(!mKeyOwner.ownsScaleKey);
        mAssignScaleRef->modifyValue(aNewData);
        notifyAssignModification(TimeKeyType_Scale);
    }
    else
    {
        cmnd::ScopedMacro macro(stack, CmndName::tr("update a scaling key"));
        setAssignNotifier(macro, TimeKeyType_Scale, mKeyOwner.ownsScaleKey);
        // push commands
        mKeyOwner.pushOwningScaleKey(stack, timeLine, frame);

        if (mSRTParam.necessarilyMove)
        {
            mKeyOwner.pushOwningMoveKey(stack, timeLine, frame);
        }
        if (mSRTParam.necessarilyRotate)
        {
            mKeyOwner.pushOwningRotateKey(stack, timeLine, frame);
        }

        mAssignScaleRef = new AssignScaleCommand(&(mKeyOwner.scaleKey->data()), aNewData);
        stack.push(mAssignScaleRef);
    }
}

} // namespace srt
} // namespace ctrl
