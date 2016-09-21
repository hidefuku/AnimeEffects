#ifndef CORE_BONE2_H
#define CORE_BONE2_H

#include <array>
#include <QVector2D>
#include "util/LifeLink.h"
#include "util/TreeNodeBase.h"
#include "util/TreeIterator.h"
#include "core/BoneShape.h"
#include "core/Serializer.h"
#include "core/Deserializer.h"
namespace core { class ObjectNode; }

namespace core
{

class Bone2 : public util::TreeNodeBase<Bone2>
{
public:
    typedef util::TreeNodeBase<Bone2>::Children ChildrenType;
    typedef util::TreeIterator<Bone2, ChildrenType::Iterator> Iterator;
    typedef util::TreeIterator<const Bone2, ChildrenType::ConstIterator> ConstIterator;

    Bone2();
    explicit Bone2(const Bone2& aRhs);
    ~Bone2();

    // for original bone
    void setWorldPos(const QVector2D& aWorldPos, const Bone2* aParent);
    void setRange(int aIndex, const QVector2D& aRange);
    void setShape(const BoneShape& aShape);
    QList<ObjectNode*>& bindingNodes();
    const QList<ObjectNode*>& bindingNodes() const;
    bool isBinding(const ObjectNode& aNode) const;

    // geometry info
    const QVector2D& localPos() const;
    float localAngle() const;
    const QVector2D& range(int aIndex) const;
    bool hasValidRange() const;
    QVector2D blendedRange(float aRate) const;
    const BoneShape& shape() const;

    // shadow bone
    Bone2* createShadow() const;

    // transform
    void setRotate(float aRotate);
    float rotate() const;

    // cache
    void updateWorldTransform();
    const QVector2D& worldPos() const;
    float worldAngle() const;

    // focus
    bool isFocused() const { return mFocus.isLinking(); }
    void setFocus(util::LifeLink& aLink) { mFocus = aLink; }

    // select
    bool isSelected() const { return mSelect.isLinking(); }
    void setSelect(util::LifeLink& aLink) { mSelect = aLink; }

    // serialize
    bool serialize(Serializer& aOut) const;
    bool deserialize(Deserializer& aIn);

private:
    // for shadow
    const Bone2* mOrigin;
    // geometry
    QVector2D mLocalPos;
    float mLocalAngle;
    std::array<QVector2D, 2> mRange;
    BoneShape mShape;
    // bind
    QList<ObjectNode*> mBindingNodes;
    // cache
    QVector2D mWorldPos;
    float mWorldAngle;
    // transform
    float mRotate;
    // focusing
    util::LifeLink::Node mFocus;
    util::LifeLink::Node mSelect;
};

} // namespace core

#endif // CORE_BONE2_H
