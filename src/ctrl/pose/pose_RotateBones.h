#ifndef CTRL_POSE_ROTATEBONES_H
#define CTRL_POSE_ROTATEBONES_H

#include "cmnd/Stable.h"
#include "core/Bone2.h"

namespace ctrl {
namespace pose {

class RotateBones : public cmnd::Stable
{
    core::Bone2* mRootTarget;
    QVector<float> mPrevs;
    QVector<float> mNexts;
    bool mDone;

public:
    RotateBones(core::Bone2* aRootTarget, const QVector<float>& aNexts);
    void modifyValue(const QVector<float>& aNexts);
    virtual void exec();
    virtual void undo();
    virtual void redo();
};

class RotateAllBones : public cmnd::Stable
{
    QList<core::Bone2*>& mTopBones;
    QVector<float> mPrevs;
    QVector<float> mNexts;
    bool mDone;

public:
    RotateAllBones(QList<core::Bone2*>& aTopBones, const QVector<float>& aNexts);
    void modifyValue(const QVector<float>& aNexts);
    virtual void exec();
    virtual void undo();
    virtual void redo();
};

} // namespace pose
} // namespace ctrl

#endif // CTRL_POSE_ROTATEBONES_H
