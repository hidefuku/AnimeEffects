#include <QDebug>
#include "XC.h"
#include "gui/tool/tool_ModePanel.h"

namespace gui {
namespace tool {

ModePanel::ModePanel(QWidget* aParent, GUIResources& aResources, const PushDelegate& aOnPushed)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mGroup(new QButtonGroup(this))
    , mButtons()
    , mLayout(this, 0, 2, 2)
    , mOnPushed(aOnPushed)
{
    this->setTitle("ToolBox");
    mGroup->setExclusive(true);
    this->setLayout(&mLayout);

    addButton(ctrl::ToolType_Cursor, "cursor", "Camera Cursor");
    addButton(ctrl::ToolType_SRT,    "srt",    "Scale Rotate Translate");
    addButton(ctrl::ToolType_Bone,   "bone",   "Bone Creating");
    addButton(ctrl::ToolType_Pose,   "pose",   "Bone Posing");
    addButton(ctrl::ToolType_Mesh,   "mesh",   "Mesh Creating");
    addButton(ctrl::ToolType_FFD,    "ffd",    "Free Form Deform");

}

void ModePanel::addButton(
        ctrl::ToolType aType, const QString& aIconName, const QString& aToolTip)
{
    QPushButton* button = new QPushButton(this);
    button->setObjectName("toolButton");
    button->setIcon(mResources.icon(aIconName));
    button->setCheckable(true);
    button->setToolTip(aToolTip);

    XC_ASSERT(aType == mButtons.size());

    mGroup->addButton(button);
    mButtons.push_back(button);
    mLayout.addWidget(button);

    this->connect(button, &QPushButton::clicked,
                  [=](bool aChecked) { this->mOnPushed(aType, aChecked); });
}

void ModePanel::pushButton(ctrl::ToolType aId)
{
    mButtons.at(aId)->click();
}

int ModePanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    int l, t, r, b;
    this->getContentsMargins(&l, &t, &r, &b);

    auto height = mLayout.heightForWidth(aWidth - l - r);
    this->setGeometry(aPos.x(), aPos.y(), aWidth, height + b);

    return aPos.y() + height + b;
}

} // namespace tool
} // namespace gui
