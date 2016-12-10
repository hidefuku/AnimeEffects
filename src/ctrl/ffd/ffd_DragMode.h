#ifndef CTRL_FFD_DRAGMODE_H
#define CTRL_FFD_DRAGMODE_H

#include "cmnd/BasicCommands.h"
#include "core/Project.h"
#include "ctrl/ffd/ffd_Target.h"
#include "ctrl/ffd/ffd_IMode.h"

namespace ctrl {
namespace ffd {

class DragMode : public IMode
{
public:
    DragMode(core::Project& aProject, Targets& aTargets);
    virtual void updateParam(const FFDParam&);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    enum State
    {
        State_Idle,
        State_Drag,
        State_Miss
    };
    void clearState();
    bool executeFocusTask(const QVector2D& aPos);
    void executeDragTask(const QVector2D& aMove);
    void assignDragging(const QVector2D& aMove);

    core::Project& mProject;
    Targets& mTargets;
    FFDParam mParam;
    State mState;
    bool mFocusing;
    Target* mFocusTarget;
    int mFocusIndex;
    QVector2D mFocusPos;
    cmnd::ModifiableAssign<gl::Vector3>* mCommandRef;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_DRAGMODE_H
