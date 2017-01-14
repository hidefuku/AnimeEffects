#ifndef GUI_PROP_ITEMBASE
#define GUI_PROP_ITEMBASE

#include <QLayout>

namespace gui {
namespace prop {

class ItemBase
{
public:
    virtual ~ItemBase() {}
    virtual QWidget* itemWidget() { return nullptr; }
    virtual QLayout* itemLayout() { return nullptr; }
    virtual void setItemEnabled(bool aEnable) = 0;
    virtual void setItemVisible(bool aVisible) = 0;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_ITEMBASE

