#ifndef CTRL_FFDEDITOR_H
#define CTRL_FFDEDITOR_H

#include "util/PlacePointer.h"
#include "util/Circle.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/RenderInfo.h"
#include "core/CameraInfo.h"
#include "core/AbstractCursor.h"
#include "core/LayerMesh.h"
#include "ctrl/IEditor.h"
#include "ctrl/DriverResources.h"
#include "ctrl/FFDParam.h"
#include "ctrl/ffd/ffd_KeyOwner.h"
#include "ctrl/ffd/ffd_MoveVertices.h"
#include "ctrl/ffd/ffd_Task.h"

namespace ctrl
{

class FFDEditor : public IEditor
{
public:
    FFDEditor(core::Project& aProject, DriverResources& aDriverResources);
    ~FFDEditor();

    virtual bool setTarget(core::ObjectNode* aTarget);

    void updateParam(const FFDParam& aParam);
    virtual bool updateCursor(
            const core::CameraInfo& aCamera,
            const core::AbstractCursor& aCursor);
    virtual void updateEvent(EventType);

    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    enum State
    {
        State_Idle,
        State_Draw,
    };

    struct Status
    {
        Status();
        void clear();
        bool hasValidBrush() const;
        bool isDrawing() const { return state == State_Draw; }
        State state;
        util::Circle brush;
        ffd::MoveVertices* commandRef;
    };

    struct Target
    {
        Target();
        Target(core::ObjectNode* aNode);
        ~Target();
        bool isValid() const;
        core::ObjectNode* node;
        ffd::KeyOwner keyOwner;
        QScopedPointer<ffd::Task> task;
    };

    core::LayerMesh* getCurrentAreaMesh(core::ObjectNode& aNode) const;
    bool resetCurrentTarget();
    void updateTargetsKeys();
    bool hasValidTarget() const;
    void clearState();
    bool executeDrawTask(const QVector2D& aCenter, const QVector2D& aMove);

    core::Project& mProject;
    DriverResources& mDriverResources;
    FFDParam mParam;

    // target info
    core::ObjectNode* mRootTarget;
    QVector<Target*> mTargets;

    // drawing state
    Status mStatus;
};

} // namespace ctrl

#endif // CTRL_FFDEDITOR_H
