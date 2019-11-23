#include "gui/InfoLabelWidget.h"

namespace gui
{

InfoLabelWidget::InfoLabelWidget(ViaPoint& aViaPoint, GUIResources& aResources, QWidget* aParent)
    : QLabel(aParent)
    , mResources(aResources)
    , mProject()
    , mIsFirstTime(true)
    , mSuspendCount(0)
{

    this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
    this->setText(""); // TODO fill with juicy info about timeline properties
}

void InfoLabelWidget::setProject(core::Project* aProject)
{
    mProject = aProject;
}

} // namespace gui
