#include "gui/PropertyWidget.h"
#include "XC.h"

#include <QResizeEvent>

namespace gui
{

PropertyWidget::PropertyWidget(ViaPoint& aViaPoint, QWidget* aParent)
    : QScrollArea(aParent)
    , mProject()
    , mTimeLineSlot()
    , mNodeAttrSlot()
    , mResModifiedSlot()
    , mTreeRestructSlot()
    , mProjAttrSlot()
    , mBoard()
{
    this->setFocusPolicy(Qt::NoFocus);

    //this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    //this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    //this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    mBoard = new prop::Backboard(aViaPoint, this);
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
        mProject->onResourceModified.disconnect(mResModifiedSlot);
        mProject->onTreeRestructured.disconnect(mTreeRestructSlot);
        mProject->onProjectAttributeModified.disconnect(mProjAttrSlot);
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

        mResModifiedSlot = aProject->onResourceModified.connect(
                    [=](core::ResourceEvent&, bool) { updateAllProperties(); });

        mTreeRestructSlot = aProject->onTreeRestructured.connect(
                    [=](core::ObjectTreeEvent&, bool) { updateAllProperties(); });

        mProjAttrSlot = aProject->onResourceModified.connect(
                    [=](core::ResourceEvent&, bool) { updateAllProperties(); });
    }

    mBoard->setProject(aProject);
}

void PropertyWidget::updateAllProperties()
{
    mBoard->updateAttribute();
    mBoard->updateKey(true, true);
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

void PropertyWidget::onKeyUpdated(core::TimeLineEvent& aEvent, bool)
{
    mBoard->updateKey(!aEvent.targets().empty(), !aEvent.defaultTargets().empty());
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
