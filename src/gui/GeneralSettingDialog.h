#ifndef GUI_GENERALSETTINGDIALOG_H
#define GUI_GENERALSETTINGDIALOG_H

#include "gui/GUIResources.h"
#include <QComboBox>
#include "gui/EasyDialog.h"
#include "core/TimeFormat.h"

namespace gui
{

class GeneralSettingDialog : public EasyDialog
{
    Q_OBJECT
public:
    GeneralSettingDialog(GUIResources& aGUIResources, QWidget* aParent);

    bool languageHasChanged();
    bool timeFormatHasChanged();
    bool themeHasChanged();
    QString theme();
private:
    void saveSettings();

    int mInitialLanguageIndex;
    QComboBox* mLanguageBox;

    int mInitialTimeFormatIndex;
    QComboBox* mTimeFormatBox;

    QString mInitialThemeKey;
    QComboBox* mThemeBox;

    GUIResources& mGUIResources;
};

} // namespace gui

#endif // GUI_GENERALSETTINGDIALOG_H
