#ifndef GUI_PROP_KEYKNOCKER_H
#define GUI_PROP_KEYKNOCKER_H

#include <functional>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>

namespace gui {
namespace prop {

class KeyKnocker : public QGroupBox
{
public:
    KeyKnocker(const QString& aLabel);
    void set(const std::function<void()>& aKnocker);

private:
    QPushButton* mButton;
    QHBoxLayout* mLayout;
};

} // namespace prop
} // namespace gui

#endif // GUI_PROP_KEYKNOCKER_H
