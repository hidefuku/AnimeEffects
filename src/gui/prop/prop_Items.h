#ifndef GUI_PROP_ITEMS_H
#define GUI_PROP_ITEMS_H

#include <functional>
#include <array>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QVector2D>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>
#include <QLineEdit>
#include "util/Easing.h"
#include "gui/prop/prop_ItemBase.h"

namespace gui {
namespace prop {

//-------------------------------------------------------------------------------------------------
class CheckItem
        : public ItemBase
{
public:
    typedef std::function<void(bool)> UpdateType;

    CheckItem(QWidget* aParent);

    QCheckBox& box() { return *mBox; }
    const QCheckBox& box() const { return *mBox; }

    bool value() const { return mBox->isChecked(); }
    void setValue(bool aValue, bool aSignal);

    virtual QWidget* itemWidget() { return mBox; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onValueUpdated;

private:
    void onEditingFinished();

    QCheckBox* mBox;
    bool mStamp;
    bool mSignal;
    bool mIsEnable;
};

//-------------------------------------------------------------------------------------------------
class ComboItem
        : public ItemBase
{
public:
    typedef std::function<void(int, int)> UpdateType;

    ComboItem(QWidget* aParent);

    QComboBox& box() { return *mBox; }
    const QComboBox& box() const { return *mBox; }

    int value() const { return mBox->currentIndex(); }
    void setValue(int aValue, bool aSignal);

    virtual QWidget* itemWidget() { return mBox; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onValueUpdated;

private:
    void onEditingFinished();

    QComboBox* mBox;
    int mStamp;
    bool mSignal;
};

//-------------------------------------------------------------------------------------------------
class Combo2DItem
        : public ItemBase
{
public:
    typedef std::function<void(QPoint, QPoint)> UpdateType;

    Combo2DItem(QWidget* aParent);

    QComboBox& box(int aIndex) { return *mBox.at(aIndex); }
    const QComboBox& box(int aIndex) const { return *mBox.at(aIndex); }

    QPoint value() const;
    void setValue(QPoint aValue, bool aSignal);

    virtual QLayout* itemLayout() { return mLayout; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onValueUpdated;

private:
    void onEditingFinished();

    QHBoxLayout* mLayout;
    std::array<QComboBox*, 2> mBox;
    QPoint mStamp;
    bool mSignal;
};

//-------------------------------------------------------------------------------------------------
class EasingItem
        : public ItemBase
{
public:
    typedef std::function<void(util::Easing::Param, util::Easing::Param)> UpdateType;

    EasingItem(QWidget* aParent);

    QComboBox& comboBox(int aIndex) { return *mBox.at(aIndex); }
    const QComboBox& comboBox(int aIndex) const { return *mBox.at(aIndex); }

    util::Easing::Param value() const;
    void setValue(util::Easing::Param aValue, bool aSignal);

    virtual QLayout* itemLayout() { return mLayout; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onValueUpdated;

private:
    void onEditingFinished();

    QLayout* mLayout;
    std::array<QComboBox*, 2> mBox;
    QDoubleSpinBox* mDBox;
    util::Easing::Param mStamp;
    bool mSignal;
};

//-------------------------------------------------------------------------------------------------
class DecimalItem
        : public ItemBase
{
public:
    typedef std::function<void(double, double)> UpdateType;

    DecimalItem(QWidget* aParent);

    QDoubleSpinBox& box() { return *mBox; }
    const QDoubleSpinBox& box() const { return *mBox; }

    double value() const { return mBox->value(); }
    void setValue(double aValue) { mBox->setValue(aValue); mStamp = aValue; }
    void setRange(double aMin, double aMax);

    virtual QWidget* itemWidget() { return mBox; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onValueUpdated;

private:
    void onEditingFinished();

    QDoubleSpinBox* mBox;
    double mStamp;
    bool mSignal;
};

//-------------------------------------------------------------------------------------------------
class Vector2DItem
        : public ItemBase
{
public:
    typedef std::function<void(QVector2D, QVector2D)> UpdateType;

    Vector2DItem(QWidget* aParent);

    QVector2D value() const;
    void setValue(QVector2D aValue);
    void setRange(float aMin, float aMax);

    virtual QLayout* itemLayout() { return mLayout; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onValueUpdated;

private:
    void onEditingFinished();

    QLayout* mLayout;
    QDoubleSpinBox* mBox[2];
    QVector2D mStamp;
    bool mSignal;
};

//-------------------------------------------------------------------------------------------------
class BrowseItem
        : public ItemBase
{
public:
    typedef std::function<void()> UpdateType;

    BrowseItem(QWidget* aParent);

    QString value() const { return mLine->text(); }
    void setValue(const QString& aValue);

    virtual QLayout* itemLayout() { return mLayout; }
    virtual void setItemEnabled(bool aEnable);

    UpdateType onButtonPushed;

private:
    QLayout* mLayout;
    QLineEdit* mLine;
    QPushButton* mButton;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_ITEMS_H
