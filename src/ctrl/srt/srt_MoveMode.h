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
    virtual bool updateCursor(const core::CameraInfo&, const core::AbstractCursor&);
    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

private:
    void clearState();
    bool assignKey(core::SRTKey::Data& aNewData);

    core::Project& mProject;
    core::ObjectNode& mTarget;
    KeyOwner& mKeyOwner;

    Symbol mSymbol;
    Symbol::FocusData mFocus;
    cmnd::ModifiableAssign<core::SRTKey::Data>* mAssignRef;
    util::PlacePointer<ScopedModifier> mSuspend;
    QVector2D mBaseVec;
    float mBaseValue;
};

} // namespace srt
} // namespace ctrl

#endif // CTRL_SRT_MOVEMODE_H
