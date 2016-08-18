#include <QLabel>
#include "gui/prop/prop_AttrGroup.h"

namespace gui {
namespace prop {

AttrGroup::AttrGroup(const QString& aTitle, int aLabelWidth)
    : QGroupBox(aTitle)
    , mLayout(new QFormLayout())
    , mLabelWidth(aLabelWidth)
    , mItems()
{
    this->setObjectName("attrGroup");

    mLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    //mLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    mLayout->setLabelAlignment(Qt::AlignRight);
    this->setLayout(mLayout);
}

AttrGroup::~AttrGroup()
{
    for (auto item : mItems)
    {
        delete item;
    }
}

void AttrGroup::addItem(const QString& aLabel, ItemBase* aItem)
{
    mItems.push_back(aItem);

    auto label = new QLabel(aLabel);
    label->setMinimumWidth(mLabelWidth);
    label->setAlignment(Qt::AlignRight);

    if (aItem->itemLayout())
    {
        mLayout->addRow(label, aItem->itemLayout());
    }
    else if (aItem->itemWidget())
    {
        mLayout->addRow(label, aItem->itemWidget());
    }
}

} // namespace prop
} // namespace gui

