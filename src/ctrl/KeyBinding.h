#ifndef CTRL_KEYBINDING_H
#define CTRL_KEYBINDING_H

#include <QString>
#include <QVariant>

namespace ctrl
{

class KeyBinding
{
    int mKeyCode;
    int mSubKeyCode;
    Qt::KeyboardModifiers mModifiers;
    static bool getKeyValidity(int aKeyCode);

public:
    KeyBinding();
    KeyBinding(int aKeyCode, Qt::KeyboardModifiers aModifiers = Qt::NoModifier, int aSubKeyCode = -1);

    int keyCode() const { return mKeyCode; }
    void setSubKeyCode(int aSubKeyCode);
    int subKeyCode() const { return mSubKeyCode; }
    bool hasSubKeyCode() const { return mSubKeyCode != -1; }
    bool isValidBinding() const;
    bool hasControlModifier() const;
    bool hasShiftModifier() const;
    bool hasAltModifier() const;
    bool hasMetaModifier() const;
    bool hasAnyModifiers() const;
    QString text() const;
    bool conflictsWith(const KeyBinding& aRhs) const;
    bool matchesExactlyWith(const KeyBinding& aRhs) const;

    void setSerialValue(const QString& aValue);
    QString serialValue() const;
};

} // namespace ctrl

#endif // CTRL_KEYBINDING_H
