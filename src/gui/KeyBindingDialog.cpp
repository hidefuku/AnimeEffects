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
    , mAllowsSubKey(false)
{
    this->setAttribute(Qt::WA_InputMethodEnabled, false); // disable IME
}

void KeyBindingDialog::KeyEdit::keyPressEvent(QKeyEvent* aEvent)
{
    QLineEdit::keyPressEvent(aEvent);

    if (!aEvent->isAutoRepeat())
    {
        auto keyCode = aEvent->key();
        const ctrl::KeyBinding keyBind(keyCode, aEvent->modifiers());

        if (keyCode == Qt::Key_Delete || keyCode == Qt::Key_Backspace)
        { // clear the binding code
            mBinding = ctrl::KeyBinding();
        }
        else
        {
            if (mAllowsSubKey && mBinding.isValidBinding() && !mBinding.hasSubKeyCode() &&
                    keyBind.isValidBinding() && !keyBind.hasAnyModifiers())
            { // assign a sub key to the previous binding code
                mBinding.setSubKeyCode(keyCode);
                mAllowsSubKey = false;
            }
            else if (keyBind.isValidBinding())
            { // assign a new binding code
                mBinding = keyBind;
                mAllowsSubKey = true;
            }
        }
        mText = mBinding.text();
    }
    mParent.updateKeyTexts();
}

void KeyBindingDialog::KeyEdit::focusOutEvent(QFocusEvent* aEvent)
{
    QLineEdit::focusOutEvent(aEvent);
    mAllowsSubKey = false;
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
    mAllowsSubKey = false;
}

//-------------------------------------------------------------------------------------------------
KeyBindingDialog::KeyBindingDialog(KeyCommandMap& aMap, QWidget* aParent)
    : EasyDialog(tr("Key Bindings"), aParent)
    , mKeyCommandMap(aMap)
    , mKeys()
    , mTabs(new QTabWidget(this))
    , mCurrentGroup()
{
    QMap<QString, QFormLayout*> groups;

    for (auto command : mKeyCommandMap.commands())
    {
        XC_PTR_ASSERT(command);

        // reserve group tab
        auto form = groups[command->group];
        if (!form)
        {
            form = createTab(command->group);
            groups[command->group] = form;
        }

        // create key editor
        auto keyEdit = new KeyEdit(*command, *this);
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
