#ifndef CTRL_DRIVER_H
#define CTRL_DRIVER_H

#include "core/Project.h"
#include "core/AbstractCursor.h"
#include "core/PenInfo.h"
#include "core/RenderInfo.h"
#include "core/ObjectNode.h"
#include "core/TimeKeyBlender.h"
#include "ctrl/ToolType.h"
#include "ctrl/IEditor.h"
#include "ctrl/SRTEditor.h"
#include "ctrl/FFDEditor.h"
#include "ctrl/BoneEditor.h"
#include "ctrl/PoseEditor.h"
#include "ctrl/MeshEditor.h"

namespace ctrl
{

class Driver
{
public:
    Driver(core::Project& aProject);

    void setTarget(core::ObjectNode* aNode);
    core::ObjectNode* currentTarget() const { return mCurrentNode; }
    void setTool(ToolType aType);
    bool updateCursor(const core::AbstractCursor& aCursor, const core::PenInfo& aPenInfo, const core::CameraInfo& aCamera);
    void updateFrame();
    void updateKey(core::TimeLineEvent& aEvent, bool aUndo);
    void updateTree(core::ObjectTreeEvent& aEvent, bool aUndo);
    void updateResource(core::ResourceEvent& aEvent, bool aUndo);
    void updateProjectAttribute();

    void renderGL(const core::RenderInfo& aRenderInfo, core::ObjectNode* aGridTarget);
    void renderQt(const core::RenderInfo& aRenderInfo, QPainter& aPainter);

    void updateParam(const SRTParam& aParam);
    void updateParam(const FFDParam& aParam);
    void updateParam(const BoneParam& aParam);
    void updateParam(const MeshParam& aParam);

private:
    void drawOutline(const core::RenderInfo& aRenderInfo, QPainter& aPainter);

    core::Project& mProject;
    ToolType mToolType;
    core::TimeKeyBlender mBlender;
    QScopedPointer<IEditor> mEditor;
    core::ObjectNode* mCurrentNode;
    int mOnUpdating;
};

} // namespace ctrl

#endif // CTRL_DRIVER_H
