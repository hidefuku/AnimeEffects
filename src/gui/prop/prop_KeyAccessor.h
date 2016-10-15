#ifndef GUI_PROP_KEYACCESSOR_H
#define GUI_PROP_KEYACCESSOR_H

#include "util/Easing.h"
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
    void assignSpline(int);
    void assignTrans(const QVector2D&);
    void assignRotate(float);
    void assignScale(const QVector2D&);

    // opa
    void assignOpaEasing(util::Easing::Param);
    void assignOpacity(float);

    // pose
    void assignPoseEasing(util::Easing::Param);

    // ffd
    void assignFFDEasing(util::Easing::Param);

    // new keys
    void knockNewSRT();
    void knockNewOpacity();
    void knockNewPose();
    void knockNewFFD();

private:
    bool isValid() const { return mProject && mTarget && mTarget->timeLine(); }

    core::Project* mProject;
    core::ObjectNode* mTarget;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_KEYACCESSOR_H
