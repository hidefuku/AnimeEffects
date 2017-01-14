#ifndef GUI_PROP_BACKBOARD_H
#define GUI_PROP_BACKBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/ViaPoint.h"
#include "gui/prop/prop_ConstantPanel.h"
#include "gui/prop/prop_DefaultKeyPanel.h"
#include "gui/prop/prop_CurrentKeyPanel.h"

namespace gui {
namespace prop {

class Backboard : public QWidget
{
public:
    Backboard(ViaPoint& aViaPoint, QWidget* aParent);
    void setProject(core::Project* aProject);
    void setTarget(core::ObjectNode* aNode);
    void setPlayBackActivity(bool aIsActive);

    void updateAttribute();
    void updateKey(bool aUpdateKey, bool aUppdateDefaultKey);
    void updateFrame();

private:
    void resetLayout();

    ViaPoint& mViaPoint;
    core::Project* mProject;
    QVBoxLayout* mLayout;
    QScopedPointer<ConstantPanel> mConstantPanel;
    QScopedPointer<DefaultKeyPanel> mDefaultKeyPanel;
    QScopedPointer<CurrentKeyPanel> mCurrentKeyPanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_BACKBOARD_H
