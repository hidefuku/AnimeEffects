#ifndef GUI_PROP_CONSTANTPANEL_H
#define GUI_PROP_CONSTANTPANEL_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/ViaPoint.h"
#include "gui/prop/prop_Panel.h"
#include "gui/prop/prop_AttrGroup.h"
#include "gui/prop/prop_Items.h"

namespace gui {
namespace prop {

class ConstantPanel : public Panel
{
public:
    ConstantPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent);
    void setTarget(core::ObjectNode* aTarget);
    void setPlayBackActivity(bool aIsActive);
    void updateAttribute();

private:
    static void assignBlendMode(core::Project&, core::ObjectNode*, img::BlendMode);
    static void assignClipped(core::Project&, core::ObjectNode*, bool);

    void build();

    ViaPoint& mViaPoint;
    core::Project& mProject;
    core::ObjectNode* mTarget;
    int mLabelWidth;

    AttrGroup* mRenderingAttributes;
    ComboItem* mBlendMode;
    CheckItem* mClipped;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_CONSTANTPANEL_H
