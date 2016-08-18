#ifndef CTRL_MESHEDITOR_H
#define CTRL_MESHEDITOR_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/AbstractCursor.h"
#include "core/MeshKey.h"
#include "ctrl/MeshParam.h"
#include "ctrl/mesh/mesh_IMode.h"
#include "ctrl/mesh/mesh_KeyOwner.h"
#include "ctrl/mesh/mesh_Target.h"
#include "ctrl/IEditor.h"

namespace ctrl
{

class MeshEditor : public IEditor
{
public:
    MeshEditor(core::Project& aProject);
    ~MeshEditor();

    virtual void setTarget(core::ObjectNode* aTarget);

    void updateParam(const MeshParam& aParam);
    virtual bool updateCursor(
            const core::CameraInfo& aCamera,
            const core::AbstractCursor& aCursor);
    virtual void updateEvent(EventType);

    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void finalize();
    void resetTarget(core::ObjectNode* aPrev, core::ObjectNode* aNext);
    void resetCurrentTarget();
    void createMode();
    void initializeKey(core::TimeLine& aLine);

    core::Project& mProject;
    MeshParam mParam;
    QScopedPointer<mesh::IMode> mCurrent;
    mesh::Target mTarget;
    mesh::KeyOwner mKeyOwner;
};

} // namespace ctrl

#endif // CTRL_MESHEDITOR_H
