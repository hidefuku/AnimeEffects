#ifndef CORE_FOLDERNODE_H
#define CORE_FOLDERNODE_H

#include <QScopedPointer>
#include "XC.h"
#include "core/ObjectNode.h"
#include "core/HeightMap.h"
#include "core/TimeLine.h"

namespace core
{

class FolderNode
        : public ObjectNode
        , public Renderer
{
public:
    FolderNode(const QString& aName);
    ~FolderNode();

    // default posture
    void setDefaultPosture(const QVector2D& aPos);
    // default depth
    void setDefaultDepth(float aValue);
    // default opacity
    void setDefaultOpacity(float aValue);

    void grabHeightMap(HeightMap* aNode);
    const HeightMap* heightMap() const { return mHeightMap.data(); }

    // from ObjectNode
    virtual ObjectType type() const { return ObjectType_Folder; }
    virtual void setName(const QString& aName) { mName = aName; }
    virtual const QString& name() const { return mName; }
    virtual float initialDepth() const;
    virtual void setVisibility(bool aIsVisible) { mIsVisible = aIsVisible; }
    virtual bool isVisible() const { return mIsVisible; }
    virtual bool canHoldChild() const { return true; }
    virtual void setInitialRect(const QRect& aRect) { mInitialRect = aRect; }
    virtual QRect initialRect() const { return mInitialRect; }
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
    virtual float renderDepth() const;
    virtual void setClipped(bool aIsClipped);
    virtual bool isClipped() const { return mIsClipped; }

private:
    void renderClippees(const RenderInfo&, const TimeCacheAccessor&);
    bool isClipper() const;

    QString mName;
    bool mIsVisible;
    QRect mInitialRect;
    QScopedPointer<HeightMap> mHeightMap;
    TimeLine mTimeLine;
    bool mIsClipped;

    std::vector<Renderer*> mClippees; // a cache for performance

};

} // namespace core

#endif // CORE_FOLDERNODE_H
