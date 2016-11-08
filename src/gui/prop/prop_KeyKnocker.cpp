#include <QLabel>
#include "gui/prop/prop_KeyKnocker.h"

namespace gui {
namespace prop {

KeyKnocker::KeyKnocker(const QString& aLabel)
    : mButton(new QPushButton())
    , mLayout(new QHBoxLayout())
{
    this->setObjectName("keyKnocker");
    mButton->setObjectName("keyKnocker");
    mButton->setFocusPolicy(Qt::NoFocus);

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->addWidget(mButton);
    mLayout->addWidget(new QLabel(aLabel));
    mLayout->addStretch();
    this->setLayout(mLayout);
}

void KeyKnocker::set(const std::function<void()>& aKnocker)
{
    mButton->connect(mButton, &QPushButton::pressed, aKnocker);
}

} // namespace prop
} // namespace gui
