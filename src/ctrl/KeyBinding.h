#ifndef CTRL_KEYBINDING_H
#define CTRL_KEYBINDING_H

#include <QString>
#include <QVariant>

namespace ctrl
{

class KeyBinding
{
    int mKeyCode;
    Qt::KeyboardModifiers mModifiers;

public:
    KeyBinding();
    KeyBinding(int aKeyCode, Qt::KeyboardModifiers aModifiers = Qt::NoModifier);

    int keyCode() const { return mKeyCode; }
    bool isValidBinding() const;
    bool hasControlModifier() const;
    bool hasShiftModifier() const;
    bool hasAltModifier() const;
    QString text() const;
    bool conflictsWith(const KeyBinding& aRhs) const;
    bool matchesExactlyWith(const KeyBinding& aRhs) const;

    void setSerialValue(const QString& aValue);
    QString serialValue() const;
};

} // namespace ctrl

#endif // CTRL_KEYBINDING_H
