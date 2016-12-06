#include <QIcon>
#include <QLabel>
#include "gui/prop/prop_KeyGroup.h"

namespace gui {
namespace prop {

KeyGroup::KeyGroup(const QString& aTitle, int aLabelWidth)
    : QGroupBox(aTitle)
    , mLabels()
    , mItems()
    , mLayout(new QFormLayout())
    , mLabelWidth(aLabelWidth)
{
    this->setObjectName("keyGroup");
    this->setFocusPolicy(Qt::NoFocus);

    mLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    //mLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    mLayout->setLabelAlignment(Qt::AlignRight);
    this->setLayout(mLayout);
    this->setCheckable(true);
    this->connect(this, &QGroupBox::clicked, this, &KeyGroup::onClicked);
}

KeyGroup::~KeyGroup()
{
    for (auto item : mItems)
    {
        delete item;
    }
}

void KeyGroup::addItem(const QString& aLabel, ItemBase* aItem)
{
    auto label = new QLabel(aLabel);
    label->setMinimumWidth(mLabelWidth);
    label->setAlignment(Qt::AlignRight);
    mLabels.push_back(label);
    mItems.push_back(aItem);

    if (aItem->itemLayout())
    {
        mLayout->addRow(label, aItem->itemLayout());
    }
    else if (aItem->itemWidget())
    {
        mLayout->addRow(label, aItem->itemWidget());
    }
}

void KeyGroup::onClicked(bool aChecked)
{
    if (aChecked)
    {
        this->setFixedHeight(QWIDGETSIZE_MAX);
    }
    else
    {
        this->setFixedHeight(20);
    }
}

} // namespace prop
} // namespace gui

