#include <QFile>
#include <QTextStream>
#include "gui/prop/prop_Panel.h"

namespace
{
static const int kCollapsedPanelHeight = 20;
}

namespace gui {
namespace prop {

Panel::Panel(const QString& aTitle, QWidget* aParent)
    : QGroupBox(aParent)
    , mLayout(new QVBoxLayout())
{
    this->setObjectName("propertyPanel");
    this->setTitle(aTitle);
    this->setCheckable(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setLayout(mLayout);

    this->connect(this, &QGroupBox::clicked, this, &Panel::onClicked);
}

/*
void Panel::addGroup(QWidget* aGroup)
{
    mLayout->addWidget(aGroup);
}
*/
void Panel::addGroup(QGroupBox* aGroup)
{
    mLayout->addWidget(aGroup);
    aGroup->connect(aGroup, &QGroupBox::clicked, this, &Panel::onChildrenClicked);
}

void Panel::addStretch()
{
    mLayout->addStretch();
}

void Panel::onClicked(bool aChecked)
{
    if (aChecked)
    {
        this->setFixedHeight(QWIDGETSIZE_MAX);
    }
    else
    {
        this->setFixedHeight(kCollapsedPanelHeight);
    }
    if (onCollapsed) onCollapsed();
}

void Panel::onChildrenClicked(bool)
{
    this->updateGeometry();
    //this->update();
}

} // namespace prop
} // namespace gui
