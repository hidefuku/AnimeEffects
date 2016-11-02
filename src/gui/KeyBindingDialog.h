#ifndef GUI_KEYBINDINGDIALOG_H
#define GUI_KEYBINDINGDIALOG_H

#include <QFormLayout>
#include <QLineEdit>
#include "ctrl/KeyBinding.h"
#include "gui/EasyDialog.h"

namespace gui
{

class KeyBindingDialog : public EasyDialog
{
public:
    KeyBindingDialog(QWidget* aParent);

private:
    class KeyEdit : public QLineEdit
    {
    public:
        KeyEdit(const QString& aCommandName, KeyBindingDialog& aParent);

        const QString& commandName() const { return mCommandName; }
        ctrl::KeyBinding& keyBinding() { return mBinding; }
        const ctrl::KeyBinding& keyBinding() const { return mBinding; }
        void setGroup(const QString& aGroup) { mGroup = aGroup; }
        const QString& group() const { return mGroup; }
        void setConflictInfo(const QString& aInfo);

    private:
        virtual void keyPressEvent(QKeyEvent* aEvent);

        KeyBindingDialog& mParent;
        QString mGroup;
        QString mCommandName;
        ctrl::KeyBinding mBinding;
    };

    QWidget* createGeneralTable();
    QWidget* createViewTable();
    QWidget* createToolsTable();
    void pushKeyEdit(QFormLayout* aLayout, KeyEdit* aKeyEdit);
    void updateConflicts();
    QString getConflictInfo(const KeyEdit& aEdit) const;

    QVector<KeyEdit*> mKeys;
    QString mCurrentGroup;
};

} // namespace gui

#endif // GUI_KEYBINDINGDIALOG_H
