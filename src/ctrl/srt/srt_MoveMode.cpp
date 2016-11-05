#include "util/MathUtil.h"
#include "cmnd/ScopedMacro.h"
#include "core/Constant.h"
#include "ctrl/TimeLineUtil.h"
#include "ctrl/srt/srt_MoveMode.h"

using namespace core;

namespace ctrl {
namespace srt {

MoveMode::MoveMode(Project& aProject, ObjectNode& aTarget, KeyOwner& aKeyOwner)
    : mProject(aProject)
    , mTarget(aTarget)
    , mKeyOwner(aKeyOwner)
    , mSymbol()
    , mFocus(FocusType_TERM, QVector2D())
    , mAssignRef()
    , mSuspend()
    , mBaseVec()
    , mBaseValue()
{
    XC_PTR_ASSERT(mTarget.timeLine());
}

bool MoveMode::updateCursor(const CameraInfo& aCamera, const AbstractCursor& aCursor)
{
    auto& key = *mKeyOwner.key;
    auto& keyMtx = mKeyOwner.mtx;

    bool mod = false;

    // update symbol
    mSymbol.build(key, keyMtx, aCamera);

    if (aCursor.emitsLeftPressedEvent())
    {
        mFocus = mSymbol.findFocus(aCursor.screenPos());
        auto focusVector = aCamera.toWorldVector(mFocus.second);
        if (mFocus.first != FocusType_TERM)
        {
            mSuspend.construct(mProject);
            mAssignRef = nullptr;

            if (mFocus.first == FocusType_Scale ||
                mFocus.first == FocusType_ScaleX ||
                mFocus.first == FocusType_ScaleY)
            {
                mBaseVec = key.scale();
                const QVector3D pos = QVector3D(aCursor.worldPos());
                const QVector2D vec = (pos - keyMtx * key.pos()).toVector2D();
                const float length = QVector2D::dotProduct(vec, focusVector);
                mBaseValue = std::max(Constant::normalizable(), length);
            }
            else if (mFocus.first == srt::FocusType_Rotate)
            {
                mBaseVec = (mKeyOwner.invSRMtx * QVector3D(focusVector)).toVector2D();
            }
            mod = true;
        }
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        SRTKey::Data newData;
        auto focusVector = aCamera.toWorldVector(mFocus.second);

        if (mFocus.first != srt::FocusType_TERM)
        {
            newData = key.data();
        }

        if (mFocus.first == srt::FocusType_Trans)
        {
            newData.pos += mKeyOwner.invSRMtx * QVector3D(aCursor.worldVel());
            newData.clampPos();
        }
        else if (mFocus.first == srt::FocusType_Rotate)
        {
            const QVector3D pos = mKeyOwner.invMtx * QVector3D(aCursor.worldPos());
            const QVector2D vec = (pos - newData.pos).toVector2D();
            const float length = vec.length();
            if (length > 1.0f)
            {
                const float rotate =
                        util::MathUtil::getAngleDifferenceRad(mBaseVec, vec);
                newData.rotate += rotate;
                newData.clampRotate();
                mBaseVec = vec.normalized();
            }
        }
        else if (mFocus.first == srt::FocusType_Scale)
        {
            const QVector3D pos = QVector3D(aCursor.worldPos());
            const QVector2D vec = (pos - keyMtx * newData.pos).toVector2D();
            const float length = QVector2D::dotProduct(vec, focusVector);
            newData.scale.setX(mBaseVec.x() * (length / mBaseValue));
            newData.scale.setY(mBaseVec.y() * (length / mBaseValue));
            newData.clampScale();
        }
        else if (mFocus.first == srt::FocusType_ScaleX)
        {
            const QVector3D pos = QVector3D(aCursor.worldPos());
            const QVector2D vec = (pos - keyMtx * newData.pos).toVector2D();
            const float length = QVector2D::dotProduct(vec, focusVector);
            newData.scale.setX(mBaseVec.x() * (length / mBaseValue));
            newData.clampScale();
        }
        else if (mFocus.first == srt::FocusType_ScaleY)
        {
            const QVector3D pos = QVector3D(aCursor.worldPos());
            const QVector2D vec = (pos - keyMtx * newData.pos).toVector2D();
            const float length = QVector2D::dotProduct(vec, focusVector);
            newData.scale.setY(mBaseVec.y() * (length / mBaseValue));
            newData.clampScale();
        }

        if (mFocus.first != srt::FocusType_TERM)
        {
            if (!assignKey(newData))
            {
                clearState();
            }
            mod = true;
        }
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        clearState();
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
    mSymbol.build(*mKeyOwner.key, mKeyOwner.mtx, aInfo.camera);
    mSymbol.draw(aInfo, aPainter, mFocus.first);
}

void MoveMode::clearState()
{
    mAssignRef = nullptr;
    mFocus.first = FocusType_TERM;
    mSuspend.destruct();
}

bool MoveMode::assignKey(SRTKey::Data& aNewData)
{
    const int frame = mProject.animator().currentFrame().get();
    TimeLine& timeLine = *mTarget.timeLine();
    cmnd::Stack& stack = mProject.commandStack();
    auto notifyType = mKeyOwner.owns() ?
                TimeLineEvent::Type_PushKey :
                TimeLineEvent::Type_ChangeKeyValue;

    if (mAssignRef && mProject.commandStack().isModifiable(mAssignRef))
    {
        XC_ASSERT(!mKeyOwner.owns());

        // modify command
        mAssignRef->modifyValue(aNewData);

        // singleshot notify
        TimeLineEvent event;
        event.setType(TimeLineEvent::Type_ChangeKeyValue);
        event.pushTarget(mTarget, TimeKeyType_SRT, frame);
        mProject.onTimeLineModified(event, false);

        return true;
    }
    else
    {
        cmnd::ScopedMacro macro(stack, "update srt");

        // set notifier
        {
            auto notifier = new TimeLineUtil::Notifier(mProject);
            notifier->event().setType(notifyType);
            notifier->event().pushTarget(mTarget, TimeKeyType_SRT, frame);
            macro.grabListener(notifier);
        }

        // push commands
        mKeyOwner.pushOwnsKey(stack, timeLine, frame);

        mAssignRef = new cmnd::ModifiableAssign<SRTKey::Data>(
                    &(mKeyOwner.key->data()), aNewData);
        stack.push(mAssignRef);

        return true;
    }

    return false;
}

} // namespace srt
} // namespace ctrl
