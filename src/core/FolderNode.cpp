#include <algorithm>
#include "core/FolderNode.h"
#include "core/ObjectNodeUtil.h"
#include "core/ObjectTreeEvent.h"
#include "core/ResourceEvent.h"
#include "core/TimeKeyExpans.h"
#include "core/ClippingFrame.h"

namespace core
{

FolderNode::FolderNode(const QString& aName)
    : mName(aName)
    , mDepth()
    , mIsVisible(true)
    , mInitialRect()
    , mHeightMap()
    , mTimeLine()
    , mIsClipped()
    , mRenderDepth()
    , mClippees()
{
}

FolderNode::~FolderNode()
{
    qDeleteAll(children());
}

void FolderNode::setDefaultPosture(const QVector2D& aPos)
{
    {
        auto key = (MoveKey*)mTimeLine.defaultKey(TimeKeyType_Move);
        if (!key)
        {
            key = new MoveKey();
            mTimeLine.grabDefaultKey(TimeKeyType_Move, key);
        }
        key->data().setPos(aPos);
    }
    {
        auto key = (RotateKey*)mTimeLine.defaultKey(TimeKeyType_Rotate);
        if (!key)
        {
            key = new RotateKey();
            mTimeLine.grabDefaultKey(TimeKeyType_Rotate, key);
        }
    }
    {
        auto key = (ScaleKey*)mTimeLine.defaultKey(TimeKeyType_Scale);
        if (!key)
        {
            key = new ScaleKey();
            mTimeLine.grabDefaultKey(TimeKeyType_Scale, key);
        }
    }
}

void FolderNode::setDefaultOpacity(float aValue)
{
    auto key = (OpaKey*)mTimeLine.defaultKey(TimeKeyType_Opa);
    if (!key)
    {
        key = new OpaKey();
        mTimeLine.grabDefaultKey(TimeKeyType_Opa, key);
    }
    key->setOpacity(aValue);
}

void FolderNode::grabHeightMap(HeightMap* aNode)
{
    mHeightMap.reset(aNode);
}

bool FolderNode::isClipper() const
{
    if (mIsClipped) return false;

    auto prev = this->prevSib();
    if (!prev || !prev->renderer() || !prev->renderer()->isClipped())
    {
        return false;
    }
    return true;
}

void FolderNode::prerender(const RenderInfo&, const TimeCacheAccessor&)
{
}

void FolderNode::render(const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    if (!mIsVisible) return;

    if (aAccessor.get(mTimeLine).opa().isZero()) return;

    if (aInfo.isGrid) return;

    // render clippees
    renderClippees(aInfo, aAccessor);
}

void FolderNode::renderClippees(
        const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    if (!aInfo.clippingFrame || !isClipper()) return;

    // reset clippees
    ObjectNodeUtil::collectRenderClippees(*this, mClippees);

    // clipping frame
    auto& frame = *aInfo.clippingFrame;

    const uint8 clippingId = frame.forwardClippingId();

    RenderInfo childInfo = aInfo;
    childInfo.clippingId = clippingId;

    uint32 stamp = frame.renderStamp() + 1;

    for (auto clippee : mClippees)
    {
        XC_PTR_ASSERT(clippee);

        // write clipper as necessary
        if (stamp != frame.renderStamp())
        {
            renderClipper(aInfo, aAccessor, clippingId);
            stamp = frame.renderStamp();
        }

        // render child
        clippee->render(childInfo, aAccessor);
    }
}

void FolderNode::renderClipper(
        const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor, uint8 aClipperId)
{
    for (auto child : this->children())
    {
        if (child->renderer())
        {
            child->renderer()->renderClipper(aInfo, aAccessor, aClipperId);
        }
    }
}

void FolderNode::setClipped(bool aIsClipped)
{
    mIsClipped = aIsClipped;
}

bool FolderNode::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'F', 'o', 'l', 'd', 'e', 'r', 'N', 'd' };

    // block begin
    auto pos = aOut.beginBlock(kSignature);

    // name
    aOut.write(mName);
    // depth
    aOut.write(mDepth);
    // visibility
    aOut.write(mIsVisible);
    // initial rect
    aOut.write(mInitialRect);
    // clipping
    aOut.write(mIsClipped);

    // timeline
    if (!mTimeLine.serialize(aOut))
    {
        return false;
    }

    // block end
    aOut.endBlock(pos);

    return !aOut.failure();
}

bool FolderNode::deserialize(Deserializer& aIn)
{
    // check block begin
    if (!aIn.beginBlock("FolderNd"))
        return aIn.errored("invalid signature of folder node");

    // name
    aIn.read(mName);
    // depth
    aIn.read(mDepth);
    // visibility
    aIn.read(mIsVisible);
    // initial rect
    aIn.read(mInitialRect);
    // clipping
    aIn.read(mIsClipped);

    // timeline
    if (!mTimeLine.deserialize(aIn))
    {
        return false;
    }

    // check block end
    if (!aIn.endBlock())
        return aIn.errored("invalid end of folder node");

    return !aIn.failure();
}

} // namespace core
