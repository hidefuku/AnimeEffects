#include "gui/tool/tool_PosePanel.h"
#include "gui/tool/tool_ItemTable.h"

namespace
{
static const int kButtonSize = 23;
static const int kButtonSpace = kButtonSize;
}

namespace gui {
namespace tool {

PosePanel::PosePanel(QWidget* aParent, GUIResources& aResources)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mParam()
    , mTypeGroup()
    , mDIWeight()
    , mEIRadius()
    , mEIPressure()
{
    this->setTitle(tr("Bone Posing"));
    createMode();
    updateTypeParam(mParam.mode);
}

void PosePanel::createMode()
{
    // type
    mTypeGroup.reset(new SingleOutItem(ctrl::PoseEditMode_TERM, QSize(kButtonSpace, kButtonSpace), this));
    mTypeGroup->setChoice(mParam.mode);
    mTypeGroup->setToolTips(QStringList() <<
                            tr("Move Bones") <<
                            tr("Pull Bones") <<
                            tr("Erase Poses"));
    mTypeGroup->setIcons(QVector<QIcon>() <<
                         mResources.icon("move") <<
                         mResources.icon("pencil") <<
                         mResources.icon("eraser"));

    mTypeGroup->connect([=](int aIndex)
    {
        this->mParam.mode = (ctrl::PoseEditMode)aIndex;
        this->updateTypeParam((ctrl::PoseEditMode)aIndex);
        this->onParamUpdated(true);
    });

    static const int kScale = 100;

    // drawing pressure
    mDIWeight.reset(new SliderItem(tr("weight"), this->palette(), this));
    mDIWeight->setAttribute(util::Range(0, kScale), mParam.diWeight * kScale, kScale / 10);
    mDIWeight->connectOnMoved([=](int aValue)
    {
        this->mParam.diWeight = (float)aValue / kScale;
        this->onParamUpdated(false);
    });
    // eraser radius
    mEIRadius.reset(new SliderItem(tr("radius"), this->palette(), this));
    mEIRadius->setAttribute(util::Range(5, 1000), mParam.eiRadius, 50);
    mEIRadius->connectOnChanged([=](int aValue)
    {
        this->mParam.eiRadius = aValue;
        this->onParamUpdated(false);
    });

    // eraser pressure
    mEIPressure.reset(new SliderItem(tr("pressure"), this->palette(), this));
    mEIPressure->setAttribute(util::Range(0, kScale), mParam.eiPressure * kScale, kScale / 10);
    mEIPressure->connectOnMoved([=](int aValue)
    {
        this->mParam.eiPressure = (float)aValue / kScale;
        this->onParamUpdated(false);
    });
}

void PosePanel::updateTypeParam(ctrl::PoseEditMode aType)
{
    const bool isDraw = aType == ctrl::PoseEditMode_Draw;
    const bool isErase = aType == ctrl::PoseEditMode_Erase;
    mDIWeight->setVisible(isDraw);
    mEIRadius->setVisible(isErase);
    mEIPressure->setVisible(isErase);
}

int PosePanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    static const int kItemLeft = 8;
    static const int kItemTop = 26;

    const int itemWidth = aWidth - kItemLeft * 2;
    QPoint curPos(kItemLeft, kItemTop);

    // type
    curPos.setY(mTypeGroup->updateGeometry(curPos, itemWidth) + curPos.y() + 5);

    if (mParam.mode == ctrl::PoseEditMode_Draw)
    {
        // weight
        curPos.setY(mDIWeight->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
    }
    else if (mParam.mode == ctrl::PoseEditMode_Erase)
    {
        // radius
        curPos.setY(mEIRadius->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
        // pressure
        curPos.setY(mEIPressure->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
    }

    // myself
    this->setGeometry(aPos.x(), aPos.y(), aWidth, curPos.y());

    return aPos.y() + curPos.y();
}

} // namespace tool
} // namespace gui

