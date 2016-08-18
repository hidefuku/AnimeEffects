#include "gui/tool/tool_ViewPanel.h"

namespace
{
static const int kButtonSize = 26;
}

namespace gui {
namespace tool {

ViewPanel::ViewPanel(QWidget* aParent, GUIResourceSet& aResources)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mButtons()
    , mLayout(this, 0, 2, 2)
{
    this->setTitle("ViewSettings");
    this->setLayout(&mLayout);
}

void ViewPanel::addButton(const QString& aIconName, bool aCheckable,
                          const QString& aToolTip, const PushDelegate& aDelegate)
{
    QPushButton* button = new QPushButton();
    button->setObjectName("viewButton");
    button->setFixedSize(QSize(kButtonSize, kButtonSize));
    button->setIcon(mResources.icon(aIconName));
    button->setIconSize(QSize(kButtonSize, kButtonSize));
    button->setCheckable(aCheckable);
    button->setToolTip(aToolTip);

    mButtons.push_back(button);
    mLayout.addWidget(button);

    this->connect(button, &QPushButton::clicked, aDelegate);
}

int ViewPanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    int l, t, r, b;
    this->getContentsMargins(&l, &t, &r, &b);

    auto height = mLayout.heightForWidth(aWidth - l - r);
    this->setGeometry(aPos.x(), aPos.y(), aWidth, height + b);

    return aPos.y() + height + b;
}

} // namespace tool
} // namespace gui
