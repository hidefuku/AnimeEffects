#include "gui/tool/tool_FFDPanel.h"
#include "gui/tool/tool_ItemTable.h"

namespace
{
static const int kButtonSize = 23;
static const int kButtonSpace = kButtonSize;
}

namespace gui {
namespace tool {

FFDPanel::FFDPanel(QWidget* aParent, GUIResources& aResources)
    : QGroupBox(aParent)
    , mResources(aResources)
    , mParam()
    , mTypeGroup()
    , mHardnessGroup()
    , mRadius()
    , mPressure()
    , mBlur()
    , mEraseRadius()
    , mErasePressure()
{
    this->setTitle("FreeFormDeform");
    createBrush();
    updateTypeParam(mParam.type);
}

void FFDPanel::createBrush()
{
    // type
    mTypeGroup.reset(new SingleOutItem(3, QSize(kButtonSpace, kButtonSpace), this));
    mTypeGroup->setChoice(mParam.type);
    mTypeGroup->setToolTips(QStringList() << "Deform Mesh" << "Erase Deforming" << "Drag Vertex");
    mTypeGroup->setIcons(QVector<QIcon>() <<
                         mResources.icon("pencil") <<
                         mResources.icon("eraser") <<
                         mResources.icon("move"));
    mTypeGroup->connect([=](int aIndex)
    {
        this->mParam.type = (ctrl::FFDParam::Type)aIndex;
        this->updateTypeParam(this->mParam.type);
        this->onParamUpdated(true);
    });

    // hardness
    mHardnessGroup.reset(new SingleOutItem(3, QSize(kButtonSpace, kButtonSpace), this));
    mHardnessGroup->setChoice(mParam.hardness);
    mHardnessGroup->setToolTips(QStringList() << "Quadratic" << "Quartic" << "Octic");
    mHardnessGroup->setIcons(QVector<QIcon>() <<
                             mResources.icon("hardness1") <<
                             mResources.icon("hardness2") <<
                             mResources.icon("hardness3"));
    mHardnessGroup->connect([=](int aIndex)
    {
        this->mParam.hardness = aIndex;
        this->onParamUpdated(false);
    });

    static const int kScale = 100;

    // radius
    mRadius.reset(new SliderItem("radius", this->palette(), this));
    mRadius->setAttribute(util::Range(5, 1000), mParam.radius, 50);
    mRadius->connectOnChanged([=](int aValue)
    {
        this->mParam.radius = aValue;
        this->onParamUpdated(false);
    });

    // pressure
    mPressure.reset(new SliderItem("pressure", this->palette(), this));
    mPressure->setAttribute(util::Range(0, kScale), mParam.pressure * kScale, kScale / 10);
    mPressure->connectOnMoved([=](int aValue)
    {
        this->mParam.pressure = (float)aValue / kScale;
        this->onParamUpdated(false);
    });

    // blur
    mBlur.reset(new SliderItem("blur", this->palette(), this));
    mBlur->setAttribute(util::Range(0, kScale), mParam.blur * kScale, kScale / 10);
    mBlur->connectOnMoved([=](int aValue)
    {
        this->mParam.blur = (float)aValue / kScale;
        this->onParamUpdated(false);
    });

    // erase radius
    mEraseRadius.reset(new SliderItem("radius", this->palette(), this));
    mEraseRadius->setAttribute(util::Range(5, 1000), mParam.eraseRadius, 50);
    mEraseRadius->connectOnChanged([=](int aValue)
    {
        this->mParam.eraseRadius = aValue;
        this->onParamUpdated(false);
    });

    // erase pressure
    mErasePressure.reset(new SliderItem("pressure", this->palette(), this));
    mErasePressure->setAttribute(util::Range(0, kScale), mParam.erasePressure * kScale, kScale / 10);
    mErasePressure->connectOnMoved([=](int aValue)
    {
        this->mParam.erasePressure = (float)aValue / kScale;
        this->onParamUpdated(false);
    });
}

void FFDPanel::updateTypeParam(ctrl::FFDParam::Type aType)
{
    const bool showPencil = (aType == ctrl::FFDParam::Type_Pencil);
    const bool showEraser = (aType == ctrl::FFDParam::Type_Eraser);

    mRadius->setVisible(showPencil);
    mPressure->setVisible(showPencil);
    mBlur->setVisible(showPencil);

    mEraseRadius->setVisible(showEraser);
    mErasePressure->setVisible(showEraser);
}

int FFDPanel::updateGeometry(const QPoint& aPos, int aWidth)
{
    static const int kItemLeft = 8;
    static const int kItemTop = 26;
    //static const int kButtonSize = 23;
    //static const int kButtonSpace = kButtonSize;
    //static const int kSliderHeight = 16;

    const int itemWidth = aWidth - kItemLeft * 2;
    QPoint curPos(kItemLeft, kItemTop);

    // type
    curPos.setY(mTypeGroup->updateGeometry(curPos, itemWidth) + curPos.y() + 5);

    // hardness
    curPos.setY(mHardnessGroup->updateGeometry(curPos, itemWidth) + curPos.y() + 5);


    if (mParam.type == ctrl::FFDParam::Type_Pencil)
    {
        // radius
        curPos.setY(mRadius->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
        // pressure
        curPos.setY(mPressure->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
        // blur
        curPos.setY(mBlur->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
    }
    else if (mParam.type == ctrl::FFDParam::Type_Eraser)
    {
        // eraseRadius
        curPos.setY(mEraseRadius->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
        // erasePressure
        curPos.setY(mErasePressure->updateGeometry(curPos, itemWidth) + curPos.y() + 5);
    }

    this->setGeometry(aPos.x(), aPos.y(), aWidth, curPos.y());

    return aPos.y() + curPos.y();
}

} // namespace tool
} // namespace gui

