#ifndef CTRL_BONEEDITOR_H
#define CTRL_BONEEDITOR_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/AbstractCursor.h"
#include "core/BoneKey.h"
#include "ctrl/BoneEditMode.h"
#include "ctrl/BoneParam.h"
#include "ctrl/bone/bone_IMode.h"
#include "ctrl/bone/bone_KeyOwner.h"
#include "ctrl/bone/bone_Target.h"
#include "ctrl/IEditor.h"
#include "ctrl/GraphicStyle.h"

namespace ctrl
{

class BoneEditor : public IEditor
{
public:
    BoneEditor(core::Project& aProject, GraphicStyle& aStyle);
    ~BoneEditor();

    virtual void setTarget(core::ObjectNode* aTarget);

    void updateParam(const BoneParam& aParam);
    virtual bool updateCursor(
            const core::CameraInfo& aCamera,
            const core::AbstractCursor& aCursor);
    virtual void updateEvent(EventType);

    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void finalize();
    void resetCurrentTarget();
    void createMode();
    void initializeKey(core::TimeLine& aLine);

    core::Project& mProject;
    GraphicStyle& mGraphicStyle;
    BoneParam mParam;
    QScopedPointer<bone::IMode> mCurrent;
    bone::Target mTarget;
    bone::KeyOwner mKeyOwner;
};

} // namespace ctrl

#endif // CTRL_BONEEDITOR_H
