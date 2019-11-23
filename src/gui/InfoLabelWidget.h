#ifndef GUI_INFOLABELWIDGET_H
#define GUI_INFOLABELWIDGET_H

#include <QLabel>
#include <QSettings>
#include "gui/GUIResources.h"
#include "gui/ViaPoint.h"
#include "core/Project.h"
#include "core/TimeInfo.h"
#include "core/TimeFormat.h"
#include "core/Animator.h"
#include "util/Range.h"

namespace gui
{

class InfoLabelWidget
        : public QLabel
{
public:
    InfoLabelWidget(GUIResources& aResources, QWidget* aParent);
    void setProject(core::Project* aProject);

    void onUpdate();

private:
    GUIResources& mResources;

    core::Project* mProject;

    QSettings mSettings;
    bool mIsFirstTime;
    int mSuspendCount;
};

} // namespace gui

#endif // GUI_INFOLABELWIDGET_H
