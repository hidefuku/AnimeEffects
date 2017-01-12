#include "cmnd/BasicCommands.h"
#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/srt/srt_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace srt {

KeyOwner::KeyOwner()
    : moveKey()
    , rotateKey()
    , scaleKey()
    , ownsMoveKey()
    , ownsRotateKey()
    , ownsScaleKey()
    , mtx()
    , invMtx()
    , invSRMtx()
    , locMtx()
    , locSRMtx()
    , hasInv()
{
}

KeyOwner::~KeyOwner()
{
    deleteOwningKeys();
}

void KeyOwner::pushOwningMoveKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame)
{
    if (ownsMoveKey)
    {
        aStack.push(new cmnd::GrabNewObject<MoveKey>(moveKey));
        aStack.push(aLine.createPusher(TimeKeyType_Move, aFrame, moveKey));
        ownsMoveKey = false;
    }
}

void KeyOwner::pushOwningRotateKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame)
{
    if (ownsRotateKey)
    {
        aStack.push(new cmnd::GrabNewObject<RotateKey>(rotateKey));
        aStack.push(aLine.createPusher(TimeKeyType_Rotate, aFrame, rotateKey));
        ownsRotateKey = false;
    }
}

void KeyOwner::pushOwningScaleKey(cmnd::Stack& aStack, core::TimeLine& aLine, int aFrame)
{
    if (ownsScaleKey)
    {
        aStack.push(new cmnd::GrabNewObject<ScaleKey>(scaleKey));
        aStack.push(aLine.createPusher(TimeKeyType_Scale, aFrame, scaleKey));
        ownsScaleKey = false;
    }
}

void KeyOwner::deleteOwningKeys()
{
    if (moveKey && ownsMoveKey) delete moveKey;
    ownsMoveKey = false;
    moveKey = nullptr;

    if (rotateKey && ownsRotateKey) delete rotateKey;
    ownsRotateKey = false;
    rotateKey = nullptr;

    if (scaleKey && ownsScaleKey) delete scaleKey;
    ownsScaleKey = false;
    scaleKey = nullptr;
}

bool KeyOwner::updatePosture(const TimeKeyExpans& aExpans)
{
    XC_PTR_ASSERT(moveKey);
    XC_PTR_ASSERT(rotateKey);
    XC_PTR_ASSERT(scaleKey);

    if (ownsMoveKey)
    {
        moveKey->setPos(aExpans.srt().pos());
        moveKey->setCentroid(aExpans.srt().centroid());
    }
    if (ownsRotateKey)
    {
        rotateKey->setRotate(aExpans.srt().rotate());
    }
    if (ownsScaleKey)
    {
        scaleKey->setScale(aExpans.srt().scale());
    }

    mtx = aExpans.srt().parentMatrix();
    invMtx = mtx.inverted(&hasInv);

    if (!hasInv)
    {
        return false;
    }

    invSRMtx = invMtx;
    invSRMtx.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

    locMtx = aExpans.srt().localMatrix();
    locSRMtx = locMtx;
    locSRMtx.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

    return true;
}

QMatrix4x4 KeyOwner::getLocalMatrixFromKeys() const
{
    if (!(bool)(*this)) return QMatrix4x4();

    SRTExpans expans;
    expans.setPos(moveKey->pos());
    expans.setRotate(rotateKey->rotate());
    expans.setScale(scaleKey->scale());
    return expans.localMatrix();
}

QMatrix4x4 KeyOwner::getLocalSRMatrixFromKeys() const
{
    if (!(bool)(*this)) return QMatrix4x4();

    SRTExpans expans;
    expans.setRotate(rotateKey->rotate());
    expans.setScale(scaleKey->scale());
    return expans.localSRMatrix();
}

} // namespace srt
} // namespace ctrl
