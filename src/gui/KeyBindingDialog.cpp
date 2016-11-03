#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QDebug>
#include <QKeyEvent>
#include <QKeySequence>
#include <QTabWidget>
#include <QFrame>
#include <QApplication>
#include "XC.h"
#include "gui/KeyBindingDialog.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
KeyBindingDialog::KeyEdit::KeyEdit(KeyCommandMap::KeyCommand& aOrigin,
                                   KeyBindingDialog& aParent)
    : QLineEdit(&aParent)
    , mParent(aParent)
    , mOrigin(aOrigin)
    , mBinding(aOrigin.binding)
    , mText(aOrigin.binding.text())
{
    this->setAttribute(Qt::WA_InputMethodEnabled, false); // disable IME
}

void KeyBindingDialog::KeyEdit::keyPressEvent(QKeyEvent* aEvent)
{
    QLineEdit::keyPressEvent(aEvent);

    const ctrl::KeyBinding keyBind(aEvent->key(), aEvent->modifiers());
    if (!aEvent->isAutoRepeat())
    {
        if (keyBind.isValidBinding())
        {
            mBinding = keyBind;
        }
        else if (aEvent->key() == Qt::Key_Delete || aEvent->key() == Qt::Key_Backspace)
        {
            mBinding = ctrl::KeyBinding();
        }
        mText = mBinding.text();
    }
    mParent.updateKeyTexts();
}

void KeyBindingDialog::KeyEdit::updateText(const QString& aConflictInfo)
{
    if (aConflictInfo.isEmpty())
    {
        this->setText(mText);
    }
    else
    {
        this->setText(mText + " (conflicted with " + aConflictInfo + ")");
    }
}

void KeyBindingDialog::KeyEdit::flushToOrigin()
{
    mOrigin.binding = mBinding;
}

//-------------------------------------------------------------------------------------------------
KeyBindingDialog::KeyBindingDialog(KeyCommandMap& aMap, QWidget* aParent)
    : EasyDialog("Key Binding Dialog", aParent)
    , mKeyCommandMap(aMap)
    , mKeys()
    , mTabs(new QTabWidget(this))
    , mCurrentGroup()
{
    QMap<QString, QFormLayout*> groups;

    for (auto itr = mKeyCommandMap.map().begin(); itr != mKeyCommandMap.map().end(); ++itr)
    {
        auto unit = itr.value();
        XC_PTR_ASSERT(unit);

        // reserve group tab
        auto form = groups[unit->group];
        if (!form)
        {
            form = createTab(unit->group);
            groups[unit->group] = form;
        }

        // create key editor
        auto keyEdit = new KeyEdit(*unit, *this);
        mKeys.push_back(keyEdit);
        form->addRow(keyEdit->label(), keyEdit);
    }

    this->setMainWidget(mTabs, false);
    this->setOkCancel([=](int aResult)->bool
    {
        if (aResult == 0)
        {
            this->saveSettings();
        }
        return true;
    });

    updateKeyTexts();
}

QFormLayout* KeyBindingDialog::createTab(const QString& aTitle)
{
    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    auto frame = new QFrame();
    scroll->setWidget(frame);

    auto form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    frame->setLayout(form);

    mTabs->addTab(scroll, aTitle);

    return form;
}

void KeyBindingDialog::updateKeyTexts()
{
    if (mKeys.empty()) return;

    for (auto itr = mKeys.begin(); itr != mKeys.end(); ++itr)
    {
        auto key1 = *itr;
        key1->updateText("");

        if (!key1->keyBinding().isValidBinding())
        {
            continue;
        }

        auto it2 = itr;
        while (it2 != mKeys.begin())
        {
            --it2;
            auto key2 = *it2;

            if (key1->keyBinding().conflictsWith(key2->keyBinding()))
            {
                key1->updateText(key2->group() + "/" + key2->label());
                key2->updateText(key1->group() + "/" + key1->label());
                break;
            }
        }
    }
}

void KeyBindingDialog::saveSettings()
{
    for (auto key : mKeys)
    {
        key->flushToOrigin();
    }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationName());
    settings.beginGroup("keybindings");
    mKeyCommandMap.writeTo(settings);
    settings.endGroup();
}

} // namespace gui
