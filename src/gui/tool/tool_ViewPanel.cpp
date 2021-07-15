#include "gui/tool/tool_ViewPanel.h"

namespace gui {
namespace tool {

ViewPanel::ViewPanel(QWidget* aParent, GUIResources& aResources, const QString& aTitle)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mButtons()
    , mLayout(this, 0, 2, 2)
{
    this->setTitle(aTitle);
    this->setLayout(&mLayout);
}

void ViewPanel::addButton(const QString& aIconName, bool aCheckable,
                          const QString& aToolTip, const PushDelegate& aDelegate)
{
    QPushButton* button = new QPushButton();
    button->setObjectName("viewButton");
    button->setIcon(mResources.icon(aIconName));
    button->setCheckable(aCheckable);
    button->setToolTip(aToolTip);
    button->setFocusPolicy(Qt::NoFocus);

    mButtons.push_back(button);
    mLayout.addWidget(button);

    this->connect(button, &QPushButton::clicked, aDelegate);
}

int ViewPanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    QMargins margins = this->contentsMargins();

    auto height = mLayout.heightForWidth(aWidth - margins.left() - margins.right());
    this->setGeometry(aPos.x(), aPos.y(), aWidth, height + margins.bottom());

    return aPos.y() + height + margins.bottom();
}

} // namespace tool
} // namespace gui
