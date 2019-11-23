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
static const int kTimeScaleFormatTypeCount = 5;

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

QString indexToTimeScaleFormat(int aIndex)
{
    switch (aIndex)
    {
    case core::TimeFormatType::TimeFormatType_Frames_From0:   return QCoreApplication::translate("GeneralSettingsDialog", "Frame number (from 0)");
    case core::TimeFormatType::TimeFormatType_Frames_From1:   return QCoreApplication::translate("GeneralSettingsDialog", "Frame number (from 1)");
    case core::TimeFormatType::TimeFormatType_Relative_FPS:   return QCoreApplication::translate("GeneralSettingsDialog", "Relative to FPS (1.0 = 60.0)");
    case core::TimeFormatType::TimeFormatType_Seconds_Frames: return QCoreApplication::translate("GeneralSettingsDialog", "Seconds + Frame");
    case core::TimeFormatType::TimeFormatType_Timecode_SMPTE: return QCoreApplication::translate("GeneralSettingsDialog", "Timecode (SMPTE) (HH:MM:SS:FF)");
    default: return "";
    }
}

}

namespace gui
{

GeneralSettingDialog::GeneralSettingDialog(QWidget* aParent)
    : EasyDialog(tr("General Settings"), aParent)
    , mInitialLanguageIndex()
    , mLanguageBox()
    , mInitialTimeScaleFormatIndex()
    , mTimeScaleFormatBox()
{
    // read current settings
    {
        QSettings settings;
        auto language = settings.value("generalsettings/language");

        if (language.isValid())
        {
            mInitialLanguageIndex = languageToIndex(language.toString());
        }

        auto timeScale = settings.value("generalsettings/ui/timescaleformat");
        if (timeScale.isValid())
        {
            mInitialTimeScaleFormatIndex = timeScale.toInt();
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

        mTimeScaleFormatBox = new QComboBox();
        for (int i = 0; i < kTimeScaleFormatTypeCount; ++i)
        {
            mTimeScaleFormatBox->addItem(indexToTimeScaleFormat(i));
        }
        mTimeScaleFormatBox->setCurrentIndex(mInitialTimeScaleFormatIndex);
        form->addRow(tr("Time scale format :"), mTimeScaleFormatBox);
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

void GeneralSettingDialog::saveSettings()
{
    QSettings settings;
    auto newLangIndex = mLanguageBox->currentIndex();
    if (mInitialLanguageIndex != newLangIndex)
    {
        settings.setValue("generalsettings/language", indexToLanguage(newLangIndex));
    }

    auto newTimeScaleFormatIndex = mTimeScaleFormatBox->currentIndex();
    settings.setValue("generalsettings/ui/timescaleformat", newTimeScaleFormatIndex);

}

} // namespace gui
