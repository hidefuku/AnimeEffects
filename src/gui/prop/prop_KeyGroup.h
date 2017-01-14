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
    void addItem(const QString& aLabel, ItemBase* aItem);
    void makeSureExpand();

private slots:
    void onClicked(bool aChecked);

private:
    void setExpansion(bool aChecked);

    QVector<QWidget*> mLabels;
    QVector<ItemBase*> mItems;
    QFormLayout* mLayout;
    int mLabelWidth;
    bool mChecked;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_KEYGROUP_H
