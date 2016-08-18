#include <QFile>
#include <QTextStream>
#include "gui/prop/prop_Panel.h"

namespace
{
static const int kCollapsedPanelHeight = 24;
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
    this->setLayout(mLayout);

    this->connect(this, &QGroupBox::clicked, this, &Panel::onClicked);
}

void Panel::addGroup(QWidget* aGroup)
{
    mLayout->addWidget(aGroup);
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

} // namespace prop
} // namespace gui
