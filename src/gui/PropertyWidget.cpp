#include "gui/PropertyWidget.h"
#include "XC.h"

#include <QResizeEvent>

namespace gui
{

PropertyWidget::PropertyWidget(QWidget* aParent)
    : QScrollArea(aParent)
    , mProject()
    , mTimeLineSlot()
    , mNodeAttrSlot()
    , mBoard()
{
    this->setFocusPolicy(Qt::NoFocus);

    //this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    //this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    //this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    mBoard = new prop::Backboard(this);
    this->setWidget(mBoard);
}

PropertyWidget::~PropertyWidget()
{
    unlinkProject();
}

void PropertyWidget::unlinkProject()
{
    if (mProject)
    {
        mProject->onTimeLineModified.disconnect(mTimeLineSlot);
        mProject->onNodeAttributeModified.disconnect(mNodeAttrSlot);
        mProject.reset();
    }
}

void PropertyWidget::setProject(core::Project* aProject)
{
    unlinkProject();

    if (aProject)
    {
        mProject = aProject->pointee();

        mTimeLineSlot = aProject->onTimeLineModified.connect(
                    this, &PropertyWidget::onKeyUpdated);

        mNodeAttrSlot = aProject->onNodeAttributeModified.connect(
                    this, &PropertyWidget::onAttributeUpdated);
    }

    mBoard->setProject(aProject);
}

void PropertyWidget::onSelectionChanged(core::ObjectNode* aRepresentNode)
{
    mBoard->setTarget(aRepresentNode);
}

void PropertyWidget::onAttributeUpdated(core::ObjectNode&, bool)
{
    mBoard->updateAttribute();
    onVisualUpdated();
}

void PropertyWidget::onKeyUpdated(core::TimeLineEvent&, bool)
{
    mBoard->updateKey();
}

void PropertyWidget::onFrameUpdated()
{
    mBoard->updateFrame();
}

void PropertyWidget::onPlayBackStateChanged(bool aIsActive)
{
    mBoard->setPlayBackActivity(aIsActive);
}

void PropertyWidget::resizeEvent(QResizeEvent* aEvent)
{
    QScrollArea::resizeEvent(aEvent);
    mBoard->resize(aEvent->size().width(), mBoard->height());
}

} // namespace gui
