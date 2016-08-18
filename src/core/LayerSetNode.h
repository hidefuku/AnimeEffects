#ifndef CORE_LAYERSETNODE_H
#define CORE_LAYERSETNODE_H

#include <QScopedPointer>
#include "XC.h"
#include "core/ObjectNode.h"
#include "core/HeightMap.h"
#include "core/TimeLine.h"

namespace core
{

class LayerSetNode
        : public ObjectNode
        , public Renderer
{
public:
    LayerSetNode(const QString& aName);
    ~LayerSetNode();

    void setBoundingRect(const QRect& aRect);
    void grabHeightMap(HeightMap* aNode);
    const HeightMap* heightMap() const { return mHeightMap.data(); }

    // from ObjectNode
    virtual ObjectType type() const { return ObjectType_LayerSet; }
    virtual void setName(const QString& aName) { mName = aName; }
    virtual const QString& name() const { return mName; }
    virtual void setDepth(float aDepth) { mDepth = aDepth; }
    virtual float depth() const { return mDepth; }
    virtual void setVisibility(bool aIsVisible) { mIsVisible = aIsVisible; }
    virtual bool isVisible() const { return mIsVisible; }
    virtual bool canHoldChild() const { return true; }
    virtual QRect initialRect() const { return mBoundingRect; }
    virtual void setInitialCenter(const QVector2D& aCenter) { mInitialCenter = aCenter; }
    virtual QVector2D initialCenter() const { return mInitialCenter; }
    virtual Renderer* renderer() { return this; }
    virtual const Renderer* renderer() const { return this; }
    virtual TimeLine* timeLine() { return &mTimeLine; }
    virtual const TimeLine* timeLine() const { return &mTimeLine; }

    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

    // from Renderer
    virtual void prerender(const RenderInfo&, const TimeCacheAccessor&);
    virtual void render(const RenderInfo&, const TimeCacheAccessor&);
    virtual void renderClipper(
            const RenderInfo&, const TimeCacheAccessor&, uint8 aClipperId);
    virtual void setRenderDepth(float aDepth) { mRenderDepth = aDepth; }
    virtual float renderDepth() const { return mRenderDepth; }
    virtual void setClipped(bool aIsClipped);
    virtual bool isClipped() const { return mIsClipped; }

private:
    void renderClippees(const RenderInfo&, const TimeCacheAccessor&);
    bool isClipper() const;

    QString mName;
    float mDepth;
    bool mIsVisible;
    QRect mBoundingRect;
    QVector2D mInitialCenter;
    QScopedPointer<HeightMap> mHeightMap;
    TimeLine mTimeLine;
    bool mIsClipped;

    float mRenderDepth;
    std::vector<Renderer*> mClippees; // a cache for performance

};

} // namespace core

#endif // CORE_LAYERSETNODE_H
