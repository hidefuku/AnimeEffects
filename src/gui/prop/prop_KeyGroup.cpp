#include <QIcon>
#include <QLabel>
#include "gui/prop/prop_KeyGroup.h"

namespace gui {
namespace prop {

KeyGroup::KeyGroup(const QString& aTitle, int aLabelWidth)
    : QGroupBox(aTitle)
    , mLabels()
    , mItems()
    , mIcon()
    , mLayout(new QFormLayout())
    , mIsKnockableGroup(false)
    , mKeyExists(false)
    , mKeyKnockable(true)
    , mFirstFlag(true)
    , mLabelWidth(aLabelWidth)
{
    this->setObjectName("keyGroup");

    mLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    //mLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    mLayout->setLabelAlignment(Qt::AlignRight);
    this->setLayout(mLayout);

    {
        mIcon = new QCheckBox(this);
        mIcon->setObjectName("keyIcon");
        QRect geo = mIcon->geometry();
        geo.setTopLeft(QPoint(4, -10));
        mIcon->setGeometry(geo);
        mIcon->setChecked(false);
        mIcon->setCheckable(false);
        mIcon->setEnabled(true);
    }
}

KeyGroup::~KeyGroup()
{
    for (auto item : mItems)
    {
        delete item;
    }
}

void KeyGroup::setKeyKnocker(const std::function<void(bool)>& aKnocker)
{
    this->connect(mIcon, &QCheckBox::clicked, aKnocker);
    mIsKnockableGroup = true;
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

void KeyGroup::setKeyExists(bool aExist)
{
    if (!mFirstFlag && mKeyExists == aExist) return;
    mFirstFlag = false;

    for (auto label : mLabels)
    {
        label->setEnabled(aExist);
    }
    for (auto item : mItems)
    {
        item->setItemEnabled(aExist);
    }

    mIcon->setChecked(aExist);
    mIcon->setEnabled(!aExist);
    mKeyExists = aExist;
    mKeyKnockable = !aExist;

    if (mIsKnockableGroup)
    {
        mIcon->setCheckable(!aExist);
    }
}

void KeyGroup::setKeyExists(bool aExist, bool aKnockable)
{
    if (!mFirstFlag && mKeyExists == aExist && mKeyKnockable == aKnockable) return;
    mFirstFlag = false;

    for (auto label : mLabels)
    {
        label->setEnabled(aExist);
    }
    for (auto item : mItems)
    {
        item->setItemEnabled(aExist);
    }

    mIcon->setChecked(aExist);
    mIcon->setEnabled(!aExist);
    mKeyExists = aExist;

    if (aExist)
    {
        mKeyKnockable = false;
    }
    else
    {
        mKeyKnockable = aKnockable;
    }

    if (mIsKnockableGroup)
    {
        mIcon->setCheckable(mKeyKnockable);
        if (aExist)
        {
            mIcon->setVisible(true);
        }
        else
        {
            mIcon->setVisible(mKeyKnockable);
        }
    }
}

bool KeyGroup::keyExists() const
{
    return mKeyExists;
}

} // namespace prop
} // namespace gui

