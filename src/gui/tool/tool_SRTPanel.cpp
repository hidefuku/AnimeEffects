#include "gui/tool/tool_SRTPanel.h"
#include "gui/tool/tool_ItemTable.h"

namespace
{
static const int kButtonSize = 23;
static const int kButtonSpace = kButtonSize;
}

namespace gui {
namespace tool {

SRTPanel::SRTPanel(QWidget* aParent, GUIResourceSet& aResources)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mParam()
    , mTypeGroup()
{
    this->setTitle("SRT Transform");
    createMode();
    updateTypeParam(mParam.mode);
}

void SRTPanel::createMode()
{
    // mode
    mTypeGroup.reset(new SingleOutItem(2, QSize(kButtonSpace, kButtonSpace), this));
    mTypeGroup->setChoice(mParam.mode);
    mTypeGroup->setToolTips(QStringList() << "Transform SRT" << "Transform Centroid");
    mTypeGroup->setIcons(QVector<QIcon>() << mResources.icon("move") << mResources.icon("transcent"));
    mTypeGroup->connect([=](int aIndex)
    {
        this->mParam.mode = aIndex;
        this->updateTypeParam(aIndex);
        this->onParamUpdated(true);
    });
}

void SRTPanel::updateTypeParam(int)
{
}

int SRTPanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    static const int kItemLeft = 8;
    static const int kItemTop = 26;

    const int itemWidth = aWidth - kItemLeft * 2;
    QPoint curPos(kItemLeft, kItemTop);

    // type
    curPos.setY(mTypeGroup->updateGeometry(curPos, itemWidth) + curPos.y() + 5);

    // myself
    this->setGeometry(aPos.x(), aPos.y(), aWidth, curPos.y());

    return aPos.y() + curPos.y();
}

} // namespace tool
} // namespace gui

