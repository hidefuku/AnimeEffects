#ifndef GUI_PROP_ITEMS_H
#define GUI_PROP_ITEMS_H

#include <functional>
#include <array>
/*
#include <QFrame>
#include <QLabel>
#include <QPalette>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVector2D>
#include <QHBoxLayout>
*/
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QVector2D>
#include <QHBoxLayout>
#include <QTreeView>
#include <QCheckBox>
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

/*
#if 0
//-------------------------------------------------------------------------------------------------
class Holder
        : public QFrame
        , public ItemBase
{
public:
    Holder(const QString& aLabel, const QPalette& aPalette, int aRowCount = 1);
    virtual ~Holder();

    virtual void updateGeometry(const QPoint& aTopLeft, int aWidth);
    virtual QWidget& itemWidget() { return *this; }
    virtual const QWidget& itemWidget() const { return *this; }

    QLabel& label() { return *mLabel; }
    const QLabel& label() const { return *mLabel; }

protected:
    QHBoxLayout& layout() { return *mLayout; }
    const QHBoxLayout& layout() const { return *mLayout; }

private:
    QHBoxLayout* mLayout;
    QLabel* mLabel;
    int mRowCount;
};

//-------------------------------------------------------------------------------------------------
template<typename tValue>
class Accessor
{
public:
    virtual tValue value() const = 0;
    virtual void setValue(tValue aValue) = 0;
};

//-------------------------------------------------------------------------------------------------
class StringItem
        : public Holder
        , public Accessor<QString>
{
    Q_OBJECT
public:
    typedef std::function<void(QString, QString)> UpdateType;

    StringItem(const QString& aLabel, const QPalette& aPalette);
    virtual void updateGeometry(const QPoint& aTopLeft, int aWidth);

    // from Accessor
    QString value() const { return mEdit->text(); }
    void setValue(QString aValue) { mEdit->setText(aValue); mStamp = aValue; }

    UpdateType onValueUpdated;

private slots:
    void onEditingFinished();

private:
    QLineEdit* mEdit;
    QString mStamp;
};

//-------------------------------------------------------------------------------------------------
class IntegerItem
        : public Holder
        , public Accessor<int>
{
    Q_OBJECT
public:
    typedef std::function<void(int, int)> UpdateType;

    IntegerItem(const QString& aLabel, const QPalette& aPalette);
    virtual void updateGeometry(const QPoint& aTopLeft, int aWidth);

    QSpinBox& box() { return *mBox; }
    const QSpinBox& box() const { return *mBox; }

    // from Accessor
    int value() const { return mBox->value(); }
    void setValue(int aValue) { mBox->setValue(aValue); mStamp = aValue; }
    void setRange(int aMin, int aMax) { mBox->setRange(aMin, aMax); }

    UpdateType onValueUpdated;

private slots:
    void onEditingFinished();

private:
    QSpinBox* mBox;
    int mStamp;
};

//-------------------------------------------------------------------------------------------------
class DecimalItem
        : public Holder
        , public Accessor<double>
{
    Q_OBJECT
public:
    typedef std::function<void(double, double)> UpdateType;

    DecimalItem(const QString& aLabel, const QPalette& aPalette);
    virtual void updateGeometry(const QPoint& aTopLeft, int aWidth);

    QDoubleSpinBox& box() { return *mBox; }
    const QDoubleSpinBox& box() const { return *mBox; }

    // from Accessor
    double value() const { return mBox->value(); }
    void setValue(double aValue) { mBox->setValue(aValue); mStamp = aValue; }
    void setRange(double aMin, double aMax) { mBox->setRange(aMin, aMax); }

    UpdateType onValueUpdated;

private slots:
    void onEditingFinished();

private:
    QDoubleSpinBox* mBox;
    double mStamp;
};

//-------------------------------------------------------------------------------------------------
class Vector2DItem
        : public Holder
        , public Accessor<QVector2D>
{
    Q_OBJECT
public:
    typedef std::function<void(QVector2D, QVector2D)> UpdateType;

    Vector2DItem(const QString& aLabel, const QPalette& aPalette);
    virtual void updateGeometry(const QPoint& aTopLeft, int aWidth);

    // from Accessor
    QVector2D value() const { return QVector2D(mBox[0]->value(), mBox[1]->value()); }
    void setValue(QVector2D aValue) { mBox[0]->setValue(aValue.x()); mBox[1]->setValue(aValue.y()); mStamp = aValue; }
    void setRange(float aMin, float aMax) { mBox[0]->setRange(aMin, aMax); mBox[1]->setRange(aMin, aMax); }

    UpdateType onValueUpdated;

private slots:
    void onEditingFinished();

private:
    QDoubleSpinBox* mBox[2];
    QVector2D mStamp;
};
#endif
*/

} // namespace prop
} // namespace gui

#endif // GUI_PROP_ITEMS_H
