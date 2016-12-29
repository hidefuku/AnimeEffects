#ifndef GUI_GENERALSETTINGDIALOG_H
#define GUI_GENERALSETTINGDIALOG_H

#include <QComboBox>
#include "gui/EasyDialog.h"

namespace gui
{

class GeneralSettingDialog : public EasyDialog
{
    Q_OBJECT
public:
    GeneralSettingDialog(QWidget* aParent);

private:
    void saveSettings();

    int mInitialLanguageIndex;
    QComboBox* mLanguageBox;
};

} // namespace gui

#endif // GUI_GENERALSETTINGDIALOG_H
