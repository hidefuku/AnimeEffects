#include <QApplication>
#include <QSettings>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include "util/SelectArgs.h"
#include "gui/GeneralSettingDialog.h"

namespace
{

static const int kLanguageTypeCount = 3;
static const int kTimeFormatTypeCount = 6;

int languageToIndex(const QString& aLanguage)
{
    if (aLanguage == "Auto") return 0;
    else if (aLanguage == "English") return 1;
    else if (aLanguage == "Japanese") return 2;
    else return 0;
}

QString indexToLanguage(int aIndex)
{
    switch (aIndex)
    {
    case 0: return "Auto";
    case 1: return "English";
    case 2: return "Japanese";
    default: return "";
    }
}

QString indexToTimeFormat(int aIndex)
{
    switch (aIndex)
    {
    case core::TimeFormatType::TimeFormatType_Frames_From0:   return QCoreApplication::translate("GeneralSettingsDialog", "Frame number (from 0)");
    case core::TimeFormatType::TimeFormatType_Frames_From1:   return QCoreApplication::translate("GeneralSettingsDialog", "Frame number (from 1)");
    case core::TimeFormatType::TimeFormatType_Relative_FPS:   return QCoreApplication::translate("GeneralSettingsDialog", "Relative to FPS (1.0 = 60.0)");
    case core::TimeFormatType::TimeFormatType_Seconds_Frames: return QCoreApplication::translate("GeneralSettingsDialog", "Seconds : Frame");
    case core::TimeFormatType::TimeFormatType_Timecode_SMPTE: return QCoreApplication::translate("GeneralSettingsDialog", "Timecode (SMPTE) (HH:MM:SS:FF)");
    case core::TimeFormatType::TimeFormatType_Timecode_HHMMSSmmm: return QCoreApplication::translate("GeneralSettingsDialog", "Timecode (HH:MM:SS:mmm)");
    default: return "";
    }
}

}

namespace gui
{

GeneralSettingDialog::GeneralSettingDialog(GUIResources &aGUIResources, QWidget* aParent)
    : EasyDialog(tr("General Settings"), aParent)
    , mInitialLanguageIndex()
    , mLanguageBox()
    , mInitialTimeFormatIndex()
    , mTimeFormatBox()
    , mInitialThemeKey("default")
    , mThemeBox()
    , mGUIResources(aGUIResources)
{
    // read current settings
    {
        QSettings settings;
        auto language = settings.value("generalsettings/language");

        if (language.isValid())
        {
            mInitialLanguageIndex = languageToIndex(language.toString());
        }

        auto timeScale = settings.value("generalsettings/ui/timeformat");
        if (timeScale.isValid())
        {
            mInitialTimeFormatIndex = timeScale.toInt();
        }

        auto theme = settings.value("generalsettings/ui/theme");
        if (theme.isValid())
        {
            mInitialThemeKey = theme.toString();
        }
    }

    auto form = new QFormLayout();
    form->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignRight);

    // create inner widgets
    {
        mLanguageBox = new QComboBox();
        for (int i = 0; i < kLanguageTypeCount; ++i)
        {
            mLanguageBox->addItem(indexToLanguage(i));
        }
        mLanguageBox->setCurrentIndex(mInitialLanguageIndex);
        form->addRow(tr("Language (needs restarting) :"), mLanguageBox);

        mTimeFormatBox = new QComboBox();
        for (int i = 0; i < kTimeFormatTypeCount; ++i)
        {
            mTimeFormatBox->addItem(indexToTimeFormat(i));
        }
        mTimeFormatBox->setCurrentIndex(mInitialTimeFormatIndex);
        form->addRow(tr("Timeline format :"), mTimeFormatBox);


        mThemeBox = new QComboBox();
        QStringList themeList = mGUIResources.themeList();
        for (int i = 0; i < themeList.size(); ++i)
        {
            mThemeBox->addItem(themeList[i], themeList[i]);
        }
        mThemeBox->setCurrentIndex(mThemeBox->findData(mInitialThemeKey));
        form->addRow(tr("Theme :"), mThemeBox);
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

bool GeneralSettingDialog::languageHasChanged()
{
    return (mInitialLanguageIndex != mLanguageBox->currentIndex());
}

bool GeneralSettingDialog::timeFormatHasChanged()
{
    return (mInitialTimeFormatIndex != mTimeFormatBox->currentIndex());
}

bool GeneralSettingDialog::themeHasChanged()
{
    return (mInitialThemeKey != mThemeBox->currentData());
}

QString GeneralSettingDialog::theme()
{
    return mThemeBox->currentData().toString();
}

void GeneralSettingDialog::saveSettings()
{
    QSettings settings;
    if (languageHasChanged())
        settings.setValue("generalsettings/language", indexToLanguage(mLanguageBox->currentIndex()));

    if(timeFormatHasChanged())
        settings.setValue("generalsettings/ui/timeformat", mTimeFormatBox->currentIndex());

    if(themeHasChanged())
        settings.setValue("generalsettings/ui/theme", mThemeBox->currentData());

}

} // namespace gui
