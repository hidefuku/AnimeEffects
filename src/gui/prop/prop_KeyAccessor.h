#ifndef GUI_PROP_KEYACCESSOR_H
#define GUI_PROP_KEYACCESSOR_H

#include "util/Easing.h"
#include "img/ResourceHandle.h"
#include "core/Project.h"
#include "core/ObjectNode.h"

namespace gui {
namespace prop {

class KeyAccessor
{
public:
    KeyAccessor();
    void setProject(core::Project* aProject);
    void setTarget(core::ObjectNode* aTarget);

    // srt
    void assignSRTEasing(util::Easing::Param);
    void assignSRTSpline(int);
    void assignSRTTrans(const QVector2D&);
    void assignSRTRotate(float);
    void assignSRTScale(const QVector2D&);

    // move
    void assignMoveEasing(util::Easing::Param);
    void assignMoveSpline(int);
    void assignMovePosition(const QVector2D&);

    // rotate
    void assignRotateEasing(util::Easing::Param);
    void assignRotateAngle(float);

    // scale
    void assignScaleEasing(util::Easing::Param);
    void assignScaleRate(const QVector2D&);

    // opa
    void assignOpaEasing(util::Easing::Param);
    void assignOpacity(float);

    // pose
    void assignPoseEasing(util::Easing::Param);

    // ffd
    void assignFFDEasing(util::Easing::Param);

    // image
    void assignImageResource(img::ResourceNode&);
    void assignImageOffset(const QVector2D&);

    // new keys
    void knockNewSRT();
    void knockNewMove();
    void knockNewRotate();
    void knockNewScale();
    void knockNewOpacity();
    void knockNewPose();
    void knockNewFFD();
    void knockNewImage(const img::ResourceHandle& aHandle);

private:
    bool isValid() const { return mProject && mTarget && mTarget->timeLine(); }
    int getFrame() const { return mProject->animator().currentFrame().get(); }

    core::Project* mProject;
    core::ObjectNode* mTarget;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_KEYACCESSOR_H
