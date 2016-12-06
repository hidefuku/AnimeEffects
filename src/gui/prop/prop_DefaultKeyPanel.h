#ifndef GUI_PROP_DEFAULTKEYPANEL_H
#define GUI_PROP_DEFAULTKEYPANEL_H

#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/ViaPoint.h"
#include "gui/prop/prop_Panel.h"
#include "gui/prop/prop_KeyGroup.h"
#include "gui/prop/prop_KeyKnocker.h"
#include "gui/prop/prop_Items.h"
#include "gui/prop/prop_KeyAccessor.h"

namespace gui {
namespace prop {

class DefaultKeyPanel : public Panel
{
public:
    DefaultKeyPanel(ViaPoint& aViaPoint, core::Project& aProject, const QString& aTitle, QWidget* aParent);
    void setTarget(core::ObjectNode* aTarget);
    void setPlayBackActivity(bool aIsActive);
    void updateKey();

private:
    class DefaultPanel
    {
    public:
        DefaultPanel(Panel& aPanel, KeyAccessor& aAccessor, int aLabelWidth);
        void setEnabled(bool);
        void setKeyValue(const core::TimeLine& aLine);

    private:
        KeyAccessor& mAccessor;
        KeyGroup* mGroup;
        DecimalItem* mDepth;
        DecimalItem* mOpacity;
    };

    void build();
    void updateKeyExists();
    void updateKeyValue();

    ViaPoint& mViaPoint;
    core::Project& mProject;
    core::ObjectNode* mTarget;
    KeyAccessor mKeyAccessor;
    int mLabelWidth;

    QScopedPointer<DefaultPanel> mDefaultPanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_DEFAULTKEYPANEL_H
