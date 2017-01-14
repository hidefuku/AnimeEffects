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
    , mChecked(true)
{
    this->setObjectName("keyGroup");
    this->setFocusPolicy(Qt::NoFocus);

    mLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    //mLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    mLayout->setFormAlignment(Qt::AlignLeft);
    mLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mLayout->setSpacing(2);
    mLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(mLayout);
    this->setCheckable(true);
    this->setChecked(mChecked);
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
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
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

void KeyGroup::makeSureExpand()
{
    setExpansion(true);
}

void KeyGroup::onClicked(bool aChecked)
{
    setExpansion(aChecked);
}

void KeyGroup::setExpansion(bool aChecked)
{
    if (mChecked != aChecked)
    {
        mChecked = aChecked;
        this->setChecked(aChecked);
        this->setFixedHeight(aChecked ? QWIDGETSIZE_MAX : 20);

        for (auto label : mLabels)
        {
            label->setVisible(aChecked);
        }
        for (auto item : mItems)
        {
            item->setItemVisible(aChecked);
        }
    }
}

} // namespace prop
} // namespace gui

