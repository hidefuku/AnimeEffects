#ifndef CTRL_SRTEDITOR_H
#define CTRL_SRTEDITOR_H

#include <QPainter>
#include "util/LifeLink.h"
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "core/RenderInfo.h"
#include "core/AbstractCursor.h"
#include "ctrl/IEditor.h"
#include "ctrl/SRTParam.h"
#include "ctrl/srt/srt_KeyOwner.h"
#include "ctrl/srt/srt_IMode.h"

namespace ctrl
{

class SRTEditor : public IEditor
{
public:
    SRTEditor(core::Project& aProject);
    ~SRTEditor();

    virtual bool setTarget(core::ObjectNode* aTarget);

    void updateParam(const SRTParam& aParam);

    virtual bool updateCursor(
            const core::CameraInfo& aCamera,
            const core::AbstractCursor& aCursor);
    virtual void updateEvent(EventType);

    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter);

    util::LifeLink::Pointee<SRTEditor> pointee() { return mLifeLink.pointee<SRTEditor>(this); }

private:
    void resetCurrentTarget();
    bool initializeKey(core::TimeLine& aLine);
    void finalize();
    void createMode();

    core::Project& mProject;
    util::LifeLink mLifeLink;

    SRTParam mParam;
    core::ObjectNode* mTarget;
    srt::KeyOwner mKeyOwner;
    QScopedPointer<srt::IMode> mCurrent;
};

} // namespace ctrl

#endif // CTRL_SRTEDITOR_H
