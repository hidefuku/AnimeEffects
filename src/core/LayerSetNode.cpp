#include <algorithm>
#include "core/LayerSetNode.h"
#include "core/ObjectNodeUtil.h"
#include "core/ObjectTreeEvent.h"
#include "core/ResourceEvent.h"
#include "core/TimeKeyExpans.h"
#include "core/ClippingFrame.h"

namespace core
{

LayerSetNode::LayerSetNode(const QString& aName)
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

LayerSetNode::~LayerSetNode()
{
    qDeleteAll(children());
}

void LayerSetNode::setDefaultPos(const QVector2D& aPos)
{
    auto key = (SRTKey*)mTimeLine.defaultKey(TimeKeyType_SRT);
    if (!key)
    {
        key = new SRTKey();
        mTimeLine.grabDefaultKey(TimeKeyType_SRT, key);
    }
    key->data().pos = QVector3D(aPos);
    key->data().clampPos();
}

void LayerSetNode::setDefaultOpacity(float aValue)
{
    auto key = (OpaKey*)mTimeLine.defaultKey(TimeKeyType_Opa);
    if (!key)
    {
        key = new OpaKey();
        mTimeLine.grabDefaultKey(TimeKeyType_Opa, key);
    }
    key->data().opacity = aValue;
    key->data().clamp();
}

void LayerSetNode::grabHeightMap(HeightMap* aNode)
{
    mHeightMap.reset(aNode);
}

bool LayerSetNode::isClipper() const
{
    if (mIsClipped) return false;

    auto prev = this->prevSib();
    if (!prev || !prev->renderer() || !prev->renderer()->isClipped())
    {
        return false;
    }
    return true;
}

void LayerSetNode::prerender(const RenderInfo&, const TimeCacheAccessor&)
{
}

void LayerSetNode::render(const RenderInfo& aInfo, const TimeCacheAccessor& aAccessor)
{
    if (!mIsVisible) return;

    if (aAccessor.get(mTimeLine).opa().isZero()) return;

    if (aInfo.isGrid) return;

    // render clippees
    renderClippees(aInfo, aAccessor);
}

void LayerSetNode::renderClippees(
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

void LayerSetNode::renderClipper(
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

void LayerSetNode::setClipped(bool aIsClipped)
{
    mIsClipped = aIsClipped;
}

bool LayerSetNode::serialize(Serializer& aOut) const
{
    static const std::array<uint8, 8> kSignature =
        { 'L', 'a', 'y', 'S', 'e', 't', 'N', 'd' };

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

bool LayerSetNode::deserialize(Deserializer& aIn)
{
    // check block begin
    if (!aIn.beginBlock("LaySetNd"))
        return aIn.errored("invalid signature of layer set node");

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
        return aIn.errored("invalid end of layer set node");

    return !aIn.failure();
}

} // namespace core
