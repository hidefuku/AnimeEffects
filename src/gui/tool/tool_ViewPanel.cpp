#include "gui/tool/tool_ViewPanel.h"

namespace gui {
namespace tool {

ViewPanel::ViewPanel(QWidget* aParent, GUIResources& aResources, const QString& aTitle)
    : QGroupBox(aParent)
    , mGUIResources(aResources)
    , mButtons()
    , mLayout(this, 0, 2, 2)
{
    this->setTitle(aTitle);
    this->setLayout(&mLayout);

    mGUIResources.onThemeChanged.connect(this, &ViewPanel::onThemeUpdated);
}

void ViewPanel::addButton(const QString& aIconName, bool aCheckable,
                          const QString& aToolTip, const PushDelegate& aDelegate)
{
    QPushButton* button = new QPushButton();
    button->setObjectName(aIconName);
    button->setIcon(mGUIResources.icon(aIconName));
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
    int l = margins.left();
    int r = margins.right();
    int b = margins.bottom();

    auto height = mLayout.heightForWidth(aWidth - l - r);
    this->setGeometry(aPos.x(), aPos.y(), aWidth, height + b);

    return aPos.y() + height + b;
}

void ViewPanel::onThemeUpdated(theme::Theme &)
{
    if(mButtons.size() > 0) {
        for (auto button : mButtons)
        {
            button->setIcon(mGUIResources.icon(button->objectName()));
        }
    }
}

} // namespace tool
} // namespace gui
