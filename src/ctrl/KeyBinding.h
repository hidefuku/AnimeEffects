#ifndef CTRL_KEYBINDING_H
#define CTRL_KEYBINDING_H

#include <QString>

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
};

} // namespace ctrl

#endif // CTRL_KEYBINDING_H
