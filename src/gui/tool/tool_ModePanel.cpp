#include "gui/tool/tool_ModePanel.h"

namespace
{
static const int kButtonSize = 26;
}

namespace gui {
namespace tool {

ModePanel::ModePanel(QWidget* aParent, GUIResources& aResources)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mGroup(new QButtonGroup(this))
    , mButtons()
    , mLayout(this, 0, 2, 2)
{
    this->setTitle("ToolBox");
    mGroup->setExclusive(true);
    this->setLayout(&mLayout);
}


void ModePanel::addButton(
        ctrl::ToolType aType, const QString& aIconName,
        const PushDelegate& aDelegate, const QString& aToolTip)
{
    QPushButton* button = new QPushButton(this);
    button->setObjectName("toolButton");
    button->setFixedSize(QSize(kButtonSize, kButtonSize));
    button->setIcon(mResources.icon(aIconName));
    button->setIconSize(QSize(kButtonSize, kButtonSize));
    button->setCheckable(true);
    button->setToolTip(aToolTip);

    mGroup->addButton(button);
    mButtons.push_back(button);
    mLayout.addWidget(button);

    const ctrl::ToolType type = aType;
    this->connect(button, &QPushButton::clicked, [=](bool aChecked)
    {
        aDelegate(type, aChecked);
    });
}

int ModePanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    int l, t, r, b;
    this->getContentsMargins(&l, &t, &r, &b);

    auto height = mLayout.heightForWidth(aWidth - l - r);
    this->setGeometry(aPos.x(), aPos.y(), aWidth, height + b);

    return aPos.y() + height + b;
}

} // namespace tool
} // namespace gui
