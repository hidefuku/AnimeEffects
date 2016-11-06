#ifndef GUI_KEYBINDINGDIALOG_H
#define GUI_KEYBINDINGDIALOG_H

#include <QFormLayout>
#include <QLineEdit>
#include <QTabWidget>
#include "ctrl/KeyBinding.h"
#include "gui/EasyDialog.h"
#include "gui/KeyCommandMap.h"

namespace gui
{

class KeyBindingDialog : public EasyDialog
{
public:
    KeyBindingDialog(KeyCommandMap& aMap, QWidget* aParent);

private:
    class KeyEdit : public QLineEdit
    {
    public:
        KeyEdit(KeyCommandMap::KeyCommand& aOrigin,
                KeyBindingDialog& aParent);

        const QString& label() const { return mOrigin.label; }
        const QString& text() const { return mText; }
        ctrl::KeyBinding& keyBinding() { return mBinding; }
        const ctrl::KeyBinding& keyBinding() const { return mBinding; }
        const QString& group() const { return mOrigin.group; }
        void updateText(const QString& aConflictInfo);
        void flushToOrigin();

    private:
        virtual void keyPressEvent(QKeyEvent* aEvent);
        virtual void focusOutEvent(QFocusEvent *event);

        KeyBindingDialog& mParent;
        KeyCommandMap::KeyCommand& mOrigin;
        ctrl::KeyBinding mBinding;
        QString mText;
        bool mAllowsSubKey;
    };

    QFormLayout* createTab(const QString& aTitle);
    void updateKeyTexts();
    void saveSettings();

    KeyCommandMap& mKeyCommandMap;
    QVector<KeyEdit*> mKeys;
    QTabWidget* mTabs;
    QString mCurrentGroup;
};

} // namespace gui

#endif // GUI_KEYBINDINGDIALOG_H
