#ifndef CTRL_POSEEDITOR_H
#define CTRL_POSEEDITOR_H

#include <QMatrix4x4>
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/AbstractCursor.h"
#include "ctrl/IEditor.h"
#include "ctrl/UILogger.h"
#include "ctrl/PoseParam.h"
#include "ctrl/pose/pose_KeyOwner.h"
#include "ctrl/pose/pose_Target.h"
namespace ctrl { namespace pose { class TransBoneMode; } }

namespace ctrl
{

class PoseEditor : public IEditor
{
public:
    PoseEditor(core::Project& aProject, UILogger& aUILogger);
    ~PoseEditor();

    virtual bool setTarget(core::ObjectNode* aTarget);

    void updateParam(const PoseParam& aParam);
    virtual bool updateCursor(
            const core::CameraInfo& aCamera,
            const core::AbstractCursor& aCursor);
    virtual void updateEvent(EventType);

    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void finalize();
    void resetCurrentTarget(QString* aMessage = nullptr);
    bool initializeKey(core::TimeLine& aLine);

    core::Project& mProject;
    UILogger& mUILogger;
    PoseParam mParam;
    pose::Target mTarget;
    pose::KeyOwner mKeyOwner;
    QScopedPointer<pose::TransBoneMode> mCurrent;
};

} // namespace ctrl

#endif // CTRL_POSEEDITOR_H
