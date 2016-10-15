#ifndef GUI_PROP_BACKBOARD_H
#define GUI_PROP_BACKBOARD_H

#include <QWidget>
#include <QVBoxLayout>
#include "core/Project.h"
#include "core/ObjectNode.h"
#include "gui/prop/prop_ProjectPanel.h"
#include "gui/prop/prop_ObjectPanel.h"

namespace gui {
namespace prop {

class Backboard : public QWidget
{
public:
    Backboard(QWidget* aParent);
    void setProject(core::Project* aProject);
    void setTarget(core::ObjectNode* aNode);
    void setPlayBackActivity(bool aIsActive);

    void updateAttribute();
    void updateKey();
    void updateFrame();

private:
    core::Project* mProject;
    QVBoxLayout* mLayout;
    QScopedPointer<ProjectPanel> mProjectPanel;
    QScopedPointer<ObjectPanel> mObjectPanel;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_BACKBOARD_H
