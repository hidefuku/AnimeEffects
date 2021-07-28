#include <QDebug>
#include "XC.h"
#include "gui/tool/tool_ModePanel.h"

namespace gui {
namespace tool {

ModePanel::ModePanel(QWidget* aParent, GUIResources& aResources, const PushDelegate& aOnPushed)
    : QGroupBox(aParent)
    , mGUIResources(aResources)
    , mGroup(new QButtonGroup(this))
    , mButtons()
    , mLayout(this, 0, 2, 2)
    , mOnPushed(aOnPushed)
{
    this->setTitle(tr("ToolBox"));
    mGroup->setExclusive(true);
    this->setLayout(&mLayout);

    addButton(ctrl::ToolType_Cursor, "cursor", tr("Camera Cursor"));
    addButton(ctrl::ToolType_SRT,    "srt",    tr("Scale Rotate Translate"));
    addButton(ctrl::ToolType_Bone,   "bone",   tr("Bone Creating"));
    addButton(ctrl::ToolType_Pose,   "pose",   tr("Bone Posing"));
    addButton(ctrl::ToolType_Mesh,   "mesh",   tr("Mesh Creating"));
    addButton(ctrl::ToolType_FFD,    "ffd",    tr("Free Form Deform"));

    mGUIResources.onThemeChanged.connect(this, &ModePanel::onThemeUpdated);
}

void ModePanel::addButton(
        ctrl::ToolType aType, const QString& aIconName, const QString& aToolTip)
{
    QPushButton* button = new QPushButton(this);
    button->setObjectName(aIconName);
    button->setIcon(mGUIResources.icon(aIconName));
    button->setCheckable(true);
    button->setToolTip(aToolTip);

    XC_ASSERT(aType == mButtons.size());

    mGroup->addButton(button);
    mButtons.push_back(button);
    mLayout.addWidget(button);

    this->connect(button, &QPushButton::clicked,
                  [=](bool aChecked) { this->mOnPushed(aType, aChecked); });
}

void ModePanel::onThemeUpdated(theme::Theme &)
{
    if(mButtons.size() > 0) {
        for (auto button : mButtons)
        {
            button->setIcon(mGUIResources.icon(button->objectName()));
        }
    }
}

void ModePanel::pushButton(ctrl::ToolType aId)
{
    mButtons.at(aId)->click();
}

int ModePanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    QMargins margins = this->contentsMargins();
    int l = margins.left();
    int r = margins.right();
    int b = margins.bottom();
 
    auto height = mLayout.heightForWidth(aWidth - l - r);
    this->setGeometry(aPos.x(), aPos.y(), aWidth, height + b);

    return aPos.y() + height + b;
}

} // namespace tool
} // namespace gui
