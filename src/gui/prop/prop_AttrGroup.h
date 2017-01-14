#ifndef GUI_PROP_ATTRGROUP_H
#define GUI_PROP_ATTRGROUP_H

#include <QGroupBox>
#include <QFormLayout>
#include "gui/prop/prop_ItemBase.h"

namespace gui {
namespace prop {

class AttrGroup : public QGroupBox
{
public:
    AttrGroup(const QString& aTitle, int aLabelWidth);
    virtual ~AttrGroup();
    void addItem(const QString& aLabel, ItemBase* aItem);

private slots:
    void onClicked(bool aChecked);

private:
    QFormLayout* mLayout;
    int mLabelWidth;
    QVector<QWidget*> mLabels;
    QVector<ItemBase*> mItems;
    bool mChecked;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_ATTRGROUP_H
