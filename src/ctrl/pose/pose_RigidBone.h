#ifndef CTRL_POSE_RIGIDBONE_H
#define CTRL_POSE_RIGIDBONE_H

#include "util/TreeNodeBase.h"
#include "util/TreeIterator.h"
#include "core/Bone2.h"

namespace ctrl {
namespace pose {

class RigidBone : public util::TreeNodeBase<RigidBone>
{
public:
    typedef util::TreeNodeBase<RigidBone>::Children ChildrenType;
    typedef util::TreeIterator<RigidBone, ChildrenType::Iterator> Iterator;
    typedef util::TreeIterator<const RigidBone, ChildrenType::ConstIterator> ConstIterator;

    RigidBone(const core::Bone2& aOrigin);

    QVector2D tailPos() const;
    QVector2D dir() const;
    void updateMotion(int aCentroid = 0);

    const core::Bone2* ptr;
    QVector2D rootPos;
    float angle;
    float length;
    QVector2D force;
    float torque;
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_RIGIDBONE_H
