#ifndef CTRL_IEDITOR
#define CTRL_IEDITOR

#include "core/Project.h"

namespace ctrl
{

class IEditor
{
public:
    enum EventType
    {
        EventType_Frame,
        EventType_TimeKey,
        EventType_Tree,
        EventType_Resource,
        EventType_ProjectAttribute
    };

    virtual ~IEditor() {}

    virtual void setTarget(core::ObjectNode* aTarget) = 0;

    virtual bool updateCursor(
            const core::CameraInfo& aCamera,
            const core::AbstractCursor& aCursor) = 0;

    virtual void updateEvent(EventType aType) = 0;

    virtual void renderQt(const core::RenderInfo& aInfo, QPainter& aPainter) = 0;
};

} // namespace ctrl

#endif // CTRL_IEDITOR

