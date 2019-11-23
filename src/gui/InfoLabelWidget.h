#ifndef GUI_INFOLABELWIDGET_H
#define GUI_INFOLABELWIDGET_H

#include <QLabel>
#include "gui/GUIResources.h"
#include "gui/ViaPoint.h"
#include "core/Project.h"
#include "core/Animator.h"

namespace gui
{

class InfoLabelWidget
        : public QLabel
{
public:
    InfoLabelWidget(ViaPoint& aViaPoint, GUIResources& aResources, QWidget* aParent);
    void setProject(core::Project* aProject);

private:
    GUIResources& mResources;

    core::Project* mProject;

    bool mIsFirstTime;
    int mSuspendCount;
};

} // namespace gui

#endif // GUI_INFOLABELWIDGET_H
