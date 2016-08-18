#ifndef CORE_OBJECTNODE_H
#define CORE_OBJECTNODE_H

#include <QRect>
#include <QRectF>
#include <list>
#include "XC.h"
#include "util/TreeNodeBase.h"
#include "util/LifeLink.h"
#include "util/TreeIterator.h"
#include "util/NonCopyable.h"
#include "cmnd/Vector.h"
#include "core/ObjectType.h"
#include "core/Renderer.h"
#include "core/ObjectNodeBox.h"
#include "core/TimeLine.h"
#include "core/GridMesh.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"
namespace core { class ObjectTreeEvent; }
namespace core { class ResourceEvent; }

namespace core
{

class ObjectNode
        : public util::TreeNodeBase<ObjectNode>
        , private util::NonCopyable
{
public:
    typedef util::TreeNodeBase<ObjectNode>::Children ChildrenType;
    typedef util::TreeIterator<ObjectNode, ChildrenType::Iterator> Iterator;
    typedef util::TreeIterator<const ObjectNode, ChildrenType::ConstIterator> ConstIterator;

    ObjectNode()
        : TreeNodeBase(this)
        , mLifeLink()
    {}

    virtual ~ObjectNode() {}

    util::LifeLink::Pointee<ObjectNode> pointee()
        { return mLifeLink.pointee<ObjectNode>(this); }

    virtual ObjectType type() const = 0;

    virtual void setName(const QString& aName) = 0;
    virtual const QString& name() const = 0;

    virtual void setDepth(float aDepth) = 0;
    virtual float depth() const = 0;

    virtual void setVisibility(bool aIsVisible) = 0;
    virtual bool isVisible() const = 0;

    virtual bool canHoldChild() const = 0;
    virtual QRect initialRect() const = 0;

    virtual void setInitialCenter(const QVector2D&) = 0;
    virtual QVector2D initialCenter() const = 0;

    virtual ObjectNodeBox* boxCache() { return NULL; }
    virtual const ObjectNodeBox* boxCache() const { return NULL; }

    virtual Renderer* renderer() { return NULL; }
    virtual const Renderer* renderer() const { return NULL; }

    virtual TimeLine* timeLine() { return NULL; }
    virtual const TimeLine* timeLine() const { return NULL; }

    virtual GridMesh* gridMesh() { return NULL; }
    virtual const GridMesh* gridMesh() const { return NULL; }

    virtual bool serialize(Serializer& aOut) const = 0;
    virtual bool deserialize(Deserializer& aIn) = 0;

    virtual cmnd::Vector createResourceUpdater(const ResourceEvent&) { return cmnd::Vector(); }

protected:
    util::LifeLink& lifeLink() { return mLifeLink; }
    const util::LifeLink& lifeLink() const { return mLifeLink; }

private:
    util::LifeLink mLifeLink;
};

} // namespace core

#endif // CORE_OBJECTNODE_H
