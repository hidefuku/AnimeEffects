#ifndef CORE_LAYERNODE_H
#define CORE_LAYERNODE_H

#include <QVector>
#include "XC.h"
#include "gl/EasyShaderProgram.h"
#include "gl/Texture.h"
#include "img/Buffer.h"
#include "img/ResourceHandle.h"
#include "core/ObjectNode.h"
#include "core/Renderer.h"
#include "core/GridMesh.h"
#include "core/HeightMap.h"
#include "core/TimeLine.h"
#include "core/BoneInfluenceMap.h"
#include "core/MeshTransformer.h"
#include "core/ShaderHolder.h"

namespace core
{

class LayerNode
        : public ObjectNode
        , public Renderer
{
public:
    LayerNode(const QString& aName, ShaderHolder& aShaderHolder);

    util::LifeLink::Pointee<LayerNode> pointee() { return lifeLink().pointee<LayerNode>(this); }

    // default image
    void setDefaultImage(const img::ResourceHandle& aHandle);
    void setDefaultImage(const img::ResourceHandle& aHandle, img::BlendMode aBlendMode);
    // default posture
    void setDefaultPosture(const QVector2D& aPos);
    // default depth
    void setDefaultDepth(float aValue);
    // default opacity
    void setDefaultOpacity(float aValue);

    // from ObjectNode
    virtual ObjectType type() const { return ObjectType_Layer; }
    virtual void setName(const QString& aName) { mName = aName; }
    virtual const QString& name() const { return mName; }
    virtual float initialDepth() const;
    virtual void setVisibility(bool aIsVisible) { mIsVisible = aIsVisible; }
    virtual bool isVisible() const { return mIsVisible; }
    virtual bool canHoldChild() const { return false; }
    virtual void setInitialRect(const QRect& aRect) { mInitialRect = aRect; }
    virtual QRect initialRect() const { return mInitialRect; }
    virtual Renderer* renderer() { return this; }
    virtual const Renderer* renderer() const { return this; }
    virtual TimeLine* timeLine() { return &mTimeLine; }
    virtual const TimeLine* timeLine() const { return &mTimeLine; }
    virtual bool hasAnyMesh() const { return true; }
    virtual bool hasAnyImage() const { return true; }

    virtual cmnd::Vector createResourceUpdater(const ResourceEvent& aEvent);

    virtual bool serialize(Serializer& aOut) const;
    virtual bool deserialize(Deserializer& aIn);

    // from Renderer
    virtual void prerender(const RenderInfo&, const TimeCacheAccessor&);
    virtual void render(const RenderInfo&, const TimeCacheAccessor&);
    virtual void renderClipper(
            const RenderInfo&, const TimeCacheAccessor&, uint8 aClipperId);
    virtual float renderDepth() const;
    virtual void setClipped(bool aIsClipped);
    virtual bool isClipped() const { return mIsClipped; }
    virtual bool hasBlendMode() const { return true; }
    virtual img::BlendMode blendMode() const;
    virtual void setBlendMode(img::BlendMode);

private:
    void transformShape(const RenderInfo& aInfo, const TimeCacheAccessor&);
    void renderShape(const RenderInfo& aInfo, const TimeCacheAccessor&);
    void renderClippees(const RenderInfo& aInfo, const TimeCacheAccessor&);
    bool isClipper() const;

    QString mName;
    bool mIsVisible;
    QRect mInitialRect;
    TimeLine mTimeLine;
    ShaderHolder& mShaderHolder;
    bool mIsClipped;

    MeshTransformer mMeshTransformer;
    LayerMesh* mCurrentMesh;
    std::vector<Renderer*> mClippees; // a cache for performance
};

} // namespace core

#endif // CORE_LAYERNODE_H
