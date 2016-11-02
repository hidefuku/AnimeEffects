#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QDebug>
#include <QKeyEvent>
#include <QKeySequence>
#include <QTabWidget>
#include <QFrame>
#include "gui/KeyBindingDialog.h"

namespace gui
{

//-------------------------------------------------------------------------------------------------
KeyBindingDialog::KeyEdit::KeyEdit(const QString& aCommandName, KeyBindingDialog& aParent)
    : QLineEdit(&aParent)
    , mParent(aParent)
    , mGroup()
    , mCommandName(aCommandName)
    , mBinding()
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
    }
    this->setText(mBinding.text());

    mParent.updateConflicts();
}

void KeyBindingDialog::KeyEdit::setConflictInfo(const QString& aInfo)
{
    if (aInfo.isEmpty())
    {
        this->setText(mBinding.text());
    }
    else
    {
        this->setText(mBinding.text() + " (conflicted with " + aInfo + ")");
    }
}

//-------------------------------------------------------------------------------------------------
KeyBindingDialog::KeyBindingDialog(QWidget* aParent)
    : EasyDialog("Key Binding Dialog", aParent)
    , mKeys()
    , mCurrentGroup()
{
    auto tab = new QTabWidget(this);
    tab->addTab(createGeneralTable(), "General");
    tab->addTab(createViewTable(), "View");
    tab->addTab(createToolsTable(), "Tools");
    this->setMainWidget(tab, false);

    this->setOkCancel();
}

QWidget* KeyBindingDialog::createGeneralTable()
{
    mCurrentGroup = "General";

    auto form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    pushKeyEdit(form, new KeyEdit("Undo a command", *this));
    pushKeyEdit(form, new KeyEdit("Redo a command", *this));

    auto scroll = new QScrollArea(this);
    //scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scroll->setWidgetResizable(true);
    auto frame = new QFrame();
    scroll->setWidget(frame);
    frame->setLayout(form);
    return scroll;
}

QWidget* KeyBindingDialog::createViewTable()
{
    mCurrentGroup = "View";

    auto form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    pushKeyEdit(form, new KeyEdit("Rotate canvas", *this));

    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto frame = new QFrame();
    scroll->setWidget(frame);
    frame->setLayout(form);
    return scroll;
}

QWidget* KeyBindingDialog::createToolsTable()
{
    mCurrentGroup = "Tools";

    auto form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    pushKeyEdit(form, new KeyEdit("Select SRT", *this));

    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto frame = new QFrame();
    scroll->setWidget(frame);
    frame->setLayout(form);
    return scroll;
}

void KeyBindingDialog::pushKeyEdit(QFormLayout* aLayout, KeyEdit* aKeyEdit)
{
    aKeyEdit->setGroup(mCurrentGroup);
    mKeys.push_back(aKeyEdit);
    aLayout->addRow(mKeys.back()->commandName(), mKeys.back());
}

void KeyBindingDialog::updateConflicts()
{
    if (mKeys.empty()) return;

    mKeys.front()->setConflictInfo("");
    auto itr = mKeys.begin();
    ++itr;

    for (; itr != mKeys.end(); ++itr)
    {
        auto key = *itr;
        key->setConflictInfo("");

        if (!key->keyBinding().isValidBinding())
        {
            continue;
        }

        auto it2 = itr;
        while (it2 != mKeys.begin())
        {
            --it2;
            auto key2 = *it2;

            if (key->keyBinding().conflictsWith(key2->keyBinding()))
            {
                key->setConflictInfo(key2->group() + "/" + key2->commandName());
                key2->setConflictInfo(key->group() + "/" + key->commandName());
                break;
            }
        }
    }
}

} // namespace gui
