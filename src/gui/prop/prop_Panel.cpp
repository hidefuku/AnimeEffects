#include <QFile>
#include <QTextStream>
#include "gui/prop/prop_Panel.h"

namespace
{
static const int kCollapsedPanelHeight = 22;
}

namespace gui {
namespace prop {

Panel::Panel(const QString& aTitle, QWidget* aParent)
    : QGroupBox(aParent)
    , mLayout(new QVBoxLayout())
    , mGroups()
    , mChecked(true)
{
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0, 0, 0, 0);

    this->setObjectName("propertyPanel");
    this->setTitle(aTitle);
    this->setCheckable(true);
    this->setChecked(mChecked);
    this->setFocusPolicy(Qt::NoFocus);
    this->setLayout(mLayout);

    this->connect(this, &QGroupBox::clicked, this, &Panel::onClicked);
}

void Panel::addGroup(QGroupBox* aGroup)
{
    mLayout->addWidget(aGroup);
    aGroup->connect(aGroup, &QGroupBox::clicked, this, &Panel::onChildrenClicked);
    mGroups.push_back(aGroup);
}

void Panel::addStretch()
{
    mLayout->addStretch();
}

void Panel::onClicked(bool aChecked)
{
    if (mChecked != aChecked)
    {
        mChecked = aChecked;
        this->setFixedHeight(aChecked ? QWIDGETSIZE_MAX : kCollapsedPanelHeight);
        if (onCollapsed) onCollapsed();
    }
}

void Panel::onChildrenClicked(bool)
{
    this->updateGeometry();
    //this->update();
}

} // namespace prop
} // namespace gui
