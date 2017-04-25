#ifndef CTRL_SRT_MOVEMODE_H
#define CTRL_SRT_MOVEMODE_H

#include "util/PlacePointer.h"
#include "cmnd/BasicCommands.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "ctrl/ScopedModifier.h"
#include "ctrl/srt/srt_IMode.h"
#include "ctrl/srt/srt_KeyOwner.h"
#include "ctrl/srt/srt_Symbol.h"

namespace ctrl {
namespace srt {

//-------------------------------------------------------------------------------------------------
class MoveMode : public IMode
{
public:
    MoveMode(core::Project& aProject, core::ObjectNode& aTarget, KeyOwner& aKey);
    virtual void updateParam(const SRTParam&);
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    typedef cmnd::ModifiableAssign<core::MoveKey::Data> AssignMoveCommand;
    typedef cmnd::ModifiableAssign<core::RotateKey::Data> AssignRotateCommand;
    typedef cmnd::ModifiableAssign<core::ScaleKey::Data> AssignScaleCommand;

    void clearState();
    void assignMoveKey(core::MoveKey::Data& aNewData);
    void assignRotateKey(core::RotateKey::Data& aNewData);
    void assignScaleKey(core::ScaleKey::Data& aNewData);

    void setAssignNotifier(cmnd::ScopedMacro& aMacro,
                           core::TimeKeyType aKeyType, bool aPushKey);
    void notifyAssignModification(core::TimeKeyType aKeyType);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;
    SRTParam mSRTParam;

    Symbol mSymbol;
    Symbol::FocusData mFocus;
    AssignMoveCommand* mAssignMoveRef;
    AssignRotateCommand* mAssignRotateRef;
    AssignScaleCommand* mAssignScaleRef;
    util::PlacePointer<ScopedModifier> mSuspend;
    QVector2D mBaseVec;
    float mBaseValue;
};

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_MOVEMODE_H
