#ifndef GUI_PROP_KEYGROUP_H
#define GUI_PROP_KEYGROUP_H

#include <functional>
#include <QGroupBox>
#include <QCheckBox>
#include <QFormLayout>
#include "gui/prop/prop_ItemBase.h"

namespace gui {
namespace prop {

class KeyGroup : public QGroupBox
{
public:
    KeyGroup(const QString& aTitle, int aLabelWidth);
    virtual ~KeyGroup();
    void setKeyKnocker(const std::function<void(bool)>& aKnocker);
    void addItem(const QString& aLabel, ItemBase* aItem);
    void setKeyExists(bool aExist);
    void setKeyExists(bool aExist, bool aKnockable);
    bool keyExists() const;

private:
    QVector<QWidget*> mLabels;
    QVector<ItemBase*> mItems;
    QCheckBox* mIcon;
    QFormLayout* mLayout;
    bool mIsKnockableGroup;
    bool mKeyExists;
    bool mKeyKnockable;
    bool mFirstFlag;
    int mLabelWidth;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_KEYGROUP_H
