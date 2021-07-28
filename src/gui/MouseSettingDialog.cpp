#include <QGroupBox>
#include <QFormLayout>
#include "gui/MouseSettingDialog.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
MouseSettingDialog::MouseSettingDialog(ViaPoint& aViaPoint, QWidget* aParent)
    : EasyDialog(tr("Mouse Settings"), aParent)
    , mViaPoint(aViaPoint)
    , mInitialValues()
    , mInvertMainViewScalingBox()
    , mInvertTimeLineScalingBox()
    , mMiddleMouseMoveCanvas()
{
    // read current settings
    mInitialValues.load();

    auto form = new QFormLayout();
    form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignRight);

    // create inner widgets
    {
        mInvertMainViewScalingBox = new QCheckBox();
        mInvertMainViewScalingBox->setChecked(mInitialValues.invertMainViewScaling);
        form->addRow(tr("invert main view scaling :"), mInvertMainViewScalingBox);

        mInvertTimeLineScalingBox = new QCheckBox();
        mInvertTimeLineScalingBox->setChecked(mInitialValues.invertTimeLineScaling);
        form->addRow(tr("invert timeline scaling :"), mInvertTimeLineScalingBox);

        mMiddleMouseMoveCanvas = new QCheckBox();
        mMiddleMouseMoveCanvas->setChecked(mInitialValues.middleMouseMoveCanvas);
        form->addRow(tr("middle mouse moves canvas :"), mMiddleMouseMoveCanvas);
    }

    auto group = new QGroupBox(tr("Parameters"));
    group->setLayout(form);
    this->setMainWidget(group);

    this->setOkCancel([=](int aResult)->bool
    {
        if (aResult == 0)
        {
            this->saveSettings();
        }
        return true;
    });
}

void MouseSettingDialog::saveSettings()
{
    MouseSetting newValues;
    newValues.invertMainViewScaling = mInvertMainViewScalingBox->isChecked();
    newValues.invertTimeLineScaling = mInvertTimeLineScalingBox->isChecked();
    newValues.middleMouseMoveCanvas = mMiddleMouseMoveCanvas->isChecked();

    if (mInitialValues != newValues)
    {
        newValues.save();
        mViaPoint.mouseSetting() = newValues;
    }
}

} // namespace gui
