#ifndef GUI_GENERALSETTINGDIALOG_H
#define GUI_GENERALSETTINGDIALOG_H

#include <QComboBox>
#include "gui/EasyDialog.h"
#include "core/TimeFormat.h"

namespace gui
{

class GeneralSettingDialog : public EasyDialog
{
    Q_OBJECT
public:
    GeneralSettingDialog(QWidget* aParent);

    bool languageHasChanged();
    bool timeFormatHasChanged();
    bool themeHasChanged();
private:
    void saveSettings();

    int mInitialLanguageIndex;
    QComboBox* mLanguageBox;

    int mInitialTimeFormatIndex;
    QComboBox* mTimeFormatBox;

    int mInitialThemeIndex;
    QComboBox* mThemeBox;
};

} // namespace gui

#endif // GUI_GENERALSETTINGDIALOG_H
