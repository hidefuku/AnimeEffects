#include <QSettings>
#include <QGroupBox>
#include <QFormLayout>
#include <QComboBox>
#include "util/SelectArgs.h"
#include "gui/GeneralSettingDialog.h"

namespace
{

static const int kLanguageTypeCount = 3;

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

}

namespace gui
{

GeneralSettingDialog::GeneralSettingDialog(QWidget* aParent)
    : EasyDialog(tr("General Settings"), aParent)
    , mInitialLanguageIndex()
    , mLanguageBox()
{
    // read current settings
    {
        QSettings settings;
        auto language = settings.value("generalsettings/language");

        if (language.isValid())
        {
            mInitialLanguageIndex = languageToIndex(language.toString());
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
        form->addRow(tr("language (needs restarting) :"), mLanguageBox);
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
    auto newLangIndex = mLanguageBox->currentIndex();
    if (mInitialLanguageIndex != newLangIndex)
    {
        QSettings settings;
        settings.setValue("generalsettings/language", indexToLanguage(newLangIndex));
    }
}

} // namespace gui
