#ifndef GUI_MOUSESETTINGDIALOG_H
#define GUI_MOUSESETTINGDIALOG_H

#include <QCheckBox>
#include "gui/EasyDialog.h"
#include "gui/ViaPoint.h"
#include "gui/MouseSetting.h"

namespace gui
{

class MouseSettingDialog : public EasyDialog
{
    Q_OBJECT
public:
    MouseSettingDialog(ViaPoint& aViaPoint, QWidget* aParent);

private:
    void saveSettings();

    ViaPoint& mViaPoint;
    MouseSetting mInitialValues;
    QCheckBox* mInvertMainViewScalingBox;
    QCheckBox* mInvertTimeLineScalingBox;
};

} // namespace gui

#endif // GUI_MOUSESETTINGDIALOG_H
