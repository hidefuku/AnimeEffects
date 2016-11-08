#include "XC.h"
#include "gui/tool/tool_Items.h"
#include "gui/tool/tool_ItemTable.h"

namespace gui {
namespace tool {

//-------------------------------------------------------------------------------------------------
SingleOutItem::SingleOutItem(int aButtonCount, const QSize& aButtonSize, QWidget* aParent)
    : mGroup()
    , mButtons()
    , mButtonSize(aButtonSize)
    , mButtonNum(aButtonCount)
{
    XC_PTR_ASSERT(aParent);
    mGroup = new QButtonGroup(aParent);
    mGroup->setExclusive(true);

    mButtons.resize(mButtonNum);

    for (int i = 0; i < mButtonNum; ++i)
    {
        mButtons[i] = new QPushButton(aParent);
        mButtons[i]->setCheckable(true);
        mButtons[i]->setFocusPolicy(Qt::NoFocus);
        mGroup->addButton(mButtons[i]);
    }
}

void SingleOutItem::setToolTips(const QStringList& aTips)
{
    int i = 0;
    for (auto tip : aTips)
    {
        if (i < mButtons.size())
        {
            mButtons.at(i)->setToolTip(tip);
            ++i;
        }
        else
        {
            break;
        }
    }
}

void SingleOutItem::setIcons(const QVector<QIcon*> &aIcons)
{
    int i = 0;
    for (auto icon : aIcons)
    {
        if (i < mButtons.size())
        {
            mButtons.at(i)->setIcon(*icon);
            mButtons.at(i)->setIconSize(mButtonSize);
            ++i;
        }
        else
        {
            break;
        }
    }
}

void SingleOutItem::setIcons(const QVector<QIcon> &aIcons)
{
    int i = 0;
    for (auto icon : aIcons)
    {
        if (i < mButtons.size())
        {
            mButtons.at(i)->setIcon(icon);
            mButtons.at(i)->setIconSize(mButtonSize);
            ++i;
        }
        else
        {
            break;
        }
    }
}

void SingleOutItem::setChoice(int aButtonIndex)
{
    mButtons.at(aButtonIndex)->setChecked(true);
}

void SingleOutItem::connect(const std::function<void(int)>& aPressed)
{
    for (int i = 0; i < mButtonNum; ++i)
    {
        mGroup->connect(mButtons[i], &QPushButton::pressed, [=](){ aPressed(i); });
    }
}

int SingleOutItem::updateGeometry(const QPoint& aPos, int aWidth)
{
    // type
    ItemTable table(aPos, aWidth, mButtonSize);
    for (auto button : mButtons)
    {
        table.pushGeometry(*button);
    }
    return table.height();
}

//-------------------------------------------------------------------------------------------------
SliderItem::SliderItem(const QString& aLabel, const QPalette& aPalette, QWidget* aParent)
    : mLabel()
    , mSlider()
    , mText(aLabel)
{
    XC_PTR_ASSERT(aParent);

    mLabel = new QLabel(aParent);
    mLabel->setPalette(aPalette);
    mLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    mSlider = new QSlider(Qt::Horizontal, aParent);
    mSlider->setFocusPolicy(Qt::NoFocus);
    mSlider->connect(mSlider, &QSlider::valueChanged, [=](int aValue)
    {
        this->updateText(aValue);
    });
}

void SliderItem::setAttribute(const util::Range& aRange, int aValue, int aPageStep, int aStep)
{
    mSlider->setRange(aRange.min(), aRange.max());
    mSlider->setSingleStep(aStep);
    mSlider->setPageStep(aPageStep);
    mSlider->setValue(aValue);
    updateText(aValue);
}

void SliderItem::connectOnChanged(const std::function<void(int)>& aValueChanged)
{
    mSlider->connect(mSlider, &QSlider::valueChanged, aValueChanged);
}

void SliderItem::connectOnMoved(const std::function<void(int)>& aSliderMoved)
{
    mSlider->connect(mSlider, &QSlider::sliderMoved, aSliderMoved);
}

int SliderItem::updateGeometry(const QPoint& aPos, int aWidth)
{
    static const int kLabelHeight = 16;
    static const int kSliderHeight = 16;
    mLabel->setGeometry(aPos.x(), aPos.y(), aWidth, kLabelHeight);
    mSlider->setGeometry(aPos.x(), aPos.y() + kLabelHeight, aWidth, kSliderHeight);
    return kLabelHeight + kSliderHeight;
}

void SliderItem::updateText(int aValue)
{
    mLabel->setText(mText + ":  " + QString::number(aValue));
}

} // namespace tool
} // namespace gui

