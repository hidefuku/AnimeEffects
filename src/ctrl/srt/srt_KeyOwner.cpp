#include "cmnd/BasicCommands.h"
#include "core/TimeKeyExpans.h"
#include "core/ObjectNodeUtil.h"
#include "ctrl/srt/srt_KeyOwner.h"

using namespace core;

namespace ctrl {
namespace srt {

KeyOwner::KeyOwner()
    : key()
    , ownsKey()
    , mtx()
    , invMtx()
    , invSRMtx()
    , locMtx()
    , locSRMtx()
    , hasInv()
{
}

void KeyOwner::pushOwnsKey(cmnd::Stack& aStack, TimeLine& aLine, int aFrame)
{
    if (ownsKey)
    {
        aStack.push(new cmnd::GrabNewObject<SRTKey>(key));
        aStack.push(aLine.createPusher(TimeKeyType_SRT, aFrame, key));
        ownsKey = false;
    }
}

void KeyOwner::deleteOwnsKey()
{
    if (key && ownsKey)
    {
        delete key;
    }
    ownsKey = false;
    key = nullptr;
}

bool KeyOwner::updatePosture(const TimeKeyExpans& aExpans)
{
    XC_PTR_ASSERT(key);

    if (ownsKey)
    {
        key->data() = aExpans.srt().data();
    }

    mtx = aExpans.srt().parentMatrix();
    invMtx = mtx.inverted(&hasInv);

    if (!hasInv)
    {
        return false;
    }

    invSRMtx = invMtx;
    invSRMtx.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

    locMtx = key->data().localMatrix();
    locSRMtx = locMtx;
    locSRMtx.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

    return true;
}

} // namespace srt
} // namespace ctrl
