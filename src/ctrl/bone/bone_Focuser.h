#ifndef CTRL_BONE_FOCUSER_H
#define CTRL_BONE_FOCUSER_H

#include <QMatrix4x4>
#include "core/CameraInfo.h"
#include "core/BoneKey.h"

namespace ctrl {
namespace bone {

class Focuser
{
public:
    Focuser();

    void setTopBones(QList<core::Bone2*>& aTopBones);
    void setFocusConnector(bool aFocus);
    void setTargetMatrix(const QMatrix4x4& aMtx);

    core::Bone2* update(const core::CameraInfo& aCamera, const QVector2D& aPos);
    void clearFocus();
    bool focusChanged() const;
    float focusRate() const { return mFocusRate; }

    void select(core::Bone2& aBone);
    core::Bone2* selectingBone();
    void clearSelection();

private:
    core::Bone2* updateImpl(const core::CameraInfo& aCamera, const QVector2D& aPos);

    QList<core::Bone2*>* mTopBones;
    util::LifeLink mFocusLink;
    util::LifeLink mSelectLink;
    core::Bone2* mLastFocus;
    bool mFocusChanged;
    bool mFocusConnector;
    float mFocusRate;
    QMatrix4x4 mTargetMtx;
};

} // namespace bone
} // namespace ctrl

#endif // CTRL_BONE_FOCUSER_H
