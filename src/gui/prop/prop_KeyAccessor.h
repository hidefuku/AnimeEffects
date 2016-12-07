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

    // default values
    void assignDefaultDepth(float);
    void assignDefaultOpacity(float);
    void assignDefaultImageResource(img::ResourceNode&);
    void assignDefaultImageOffset(const QVector2D&);
    void assignDefaultImageCellSize(int);

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

    // depth
    void assignDepthEasing(util::Easing::Param);
    void assignDepthPosition(float);

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
    void knockNewMove();
    void knockNewRotate();
    void knockNewScale();
    void knockNewDepth();
    void knockNewOpacity();
    void knockNewPose();
    void knockNewFFD();
    void knockNewImage(const img::ResourceHandle& aHandle);

private:
    bool isValid() const { return mProject && mTarget && mTarget->timeLine(); }
    int getFrame() const { return mProject->animator().currentFrame().get(); }
    core::TimeLine& currline() { return *mTarget->timeLine(); }

    core::Project* mProject;
    core::ObjectNode* mTarget;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_KEYACCESSOR_H
