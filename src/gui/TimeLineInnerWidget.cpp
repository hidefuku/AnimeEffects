#include <QMenu>
#include <QMessageBox>
#include "gui/TimeLineInnerWidget.h"
#include "gui/MouseSetting.h"
#include "gui/obj/obj_Item.h"

namespace gui
{
//-------------------------------------------------------------------------------------------------
TimeLineInnerWidget::TimeCursor::TimeCursor(QWidget* aParent)
    : QWidget(aParent)
{
    this->setAutoFillBackground(false);
}

void TimeLineInnerWidget::TimeCursor::setCursorPos(const QPoint& aPos, int aHeight)
{
    const QPoint range(5, 5);
    QRect rect;
    rect.setTopLeft(aPos - range);
    rect.setBottomRight(aPos + range + QPoint(0, aHeight));
    this->setGeometry(rect);
}

void TimeLineInnerWidget::TimeCursor::paintEvent(QPaintEvent* aEvent)
{
    (void)aEvent;
    QPainter painter;
    painter.begin(this);

    const QPoint range(5, 5);
    const QBrush kBrushBody(QColor(230, 230, 230, 180));
    const QBrush kBrushEdge(QColor(80, 80, 80, 180));

    painter.setPen(QPen(kBrushEdge, 1));
    painter.setBrush(kBrushBody);
    painter.drawLine(range + QPoint(0, range.y()), range + QPoint(0, this->geometry().height()));

    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawEllipse(QPointF(range), range.x() - 0.5f, range.y() - 0.5f);

    painter.end();
}

//-------------------------------------------------------------------------------------------------
TimeLineInnerWidget::TimeLineInnerWidget(ViaPoint& aViaPoint, QWidget* aParent)
    : QWidget(aParent)
    , mViaPoint(aViaPoint)
    , mProject()
    , mTimeLineSlot()
    , mTreeRestructSlot()
    , mProjectAttrSlot()
    , mEditor()
    , mCamera()
    , mTimeCursor(this)
    , mCopyKey()
    , mPasteKey()
    , mDeleteKey()
    , mTargets()
    , mCopyTargets()
    , mPastePos()
    , mOnPasting()
{
    mTimeCursor.show();

    mEditor.reset(new ctrl::TimeLineEditor());
    this->setMouseTracking(true);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    this->connect(
                this, &QWidget::customContextMenuRequested,
                this, &TimeLineInnerWidget::onContextMenuRequested);

    {
        mCopyKey = new QAction(tr("copy key"), this);
        mCopyKey->connect(mCopyKey, &QAction::triggered, this, &TimeLineInnerWidget::onCopyKeyTriggered);

        mPasteKey = new QAction(tr("paste key"), this);
        mPasteKey->connect(mPasteKey, &QAction::triggered, this, &TimeLineInnerWidget::onPasteKeyTriggered);

        mDeleteKey = new QAction(tr("delete key"), this);
        mDeleteKey->connect(mDeleteKey, &QAction::triggered, this, &TimeLineInnerWidget::onDeleteKeyTriggered);
    }
}

void TimeLineInnerWidget::setProject(core::Project* aProject)
{
    if (mProject)
    {
        mProject->onTimeLineModified.disconnect(mTimeLineSlot);
        mProject->onTreeRestructured.disconnect(mTreeRestructSlot);
        mProject->onProjectAttributeModified.disconnect(mProjectAttrSlot);
    }

    if (aProject)
    {
        mProject = aProject->pointee();
        mTimeLineSlot = aProject->onTimeLineModified.connect(
                    this, &TimeLineInnerWidget::onTimeLineModified);

        mTreeRestructSlot = aProject->onTreeRestructured.connect(
                    this, &TimeLineInnerWidget::onTreeRestructured);

        mProjectAttrSlot = aProject->onProjectAttributeModified.connect(
                    this, &TimeLineInnerWidget::onProjectAttributeModified);
    }
    else
    {
        mProject.reset();
    }

    mEditor->setProject(aProject);
    updateSize();
}

void TimeLineInnerWidget::setFrame(core::Frame aFrame)
{
    mEditor->setFrame(aFrame);

    // particial rendering
    updateTimeCursorPos();
}

core::Frame TimeLineInnerWidget::currentFrame() const
{
    return mEditor->currentFrame();
}

int TimeLineInnerWidget::maxFrame() const
{
    return mEditor->maxFrame();
}

void TimeLineInnerWidget::updateTimeCursorPos()
{
    if (mCamera)
    {
        auto pos = mEditor->currentTimeCursorPos();
        mTimeCursor.setCursorPos(
                    QPoint(pos.x(), pos.y() - (int)mCamera->leftTopPos().y()),
                    mCamera->screenHeight());
    }
}

void TimeLineInnerWidget::updateCamera(const core::CameraInfo& aCamera)
{
    mCamera = &aCamera;
    updateSize();
    updateTimeCursorPos();
}

void TimeLineInnerWidget::updateLines(QTreeWidgetItem* aTopNode)
{
    mEditor->clearRows();

    if (!aTopNode || !aTopNode->isExpanded()) return;

    for (int i = 0; i < aTopNode->childCount(); ++i)
    {
        updateLinesRecursive(aTopNode->child(i));
    }
    updateSize();
}

void TimeLineInnerWidget::updateLinesRecursive(QTreeWidgetItem* aItem)
{
    if (!aItem) return;

    const bool isClosedFolder = !aItem->isExpanded() && aItem->childCount() > 0;
    obj::Item* objItem = obj::Item::cast(aItem);
    if (objItem && !objItem->isTopNode())
    {
        int screenTop = mCamera ? -mCamera->leftTopPos().y() : 0;

        const QRect vrect = objItem->visualRect();
        const int t = vrect.top() + screenTop;
        const int b = vrect.bottom() + screenTop;

        mEditor->pushRow(&objItem->node(), util::Range(t, b), isClosedFolder);
    }

    if (!aItem->isExpanded()) return;

    for (int i = 0; i < aItem->childCount(); ++i)
    {
        updateLinesRecursive(aItem->child(i));
    }
}

void TimeLineInnerWidget::updateLineSelection(core::ObjectNode* aRepresent)
{
    mEditor->updateRowSelection(aRepresent);
    this->update();
}

bool TimeLineInnerWidget::updateCursor(const core::AbstractCursor& aCursor)
{
    ctrl::TimeLineEditor::UpdateFlags flags = mEditor->updateCursor(aCursor);

    if (flags & ctrl::TimeLineEditor::UpdateFlag_ModView)
    {
        updateTimeCursorPos();
        this->update();
    }
    return (flags & ctrl::TimeLineEditor::UpdateFlag_ModFrame);
}

void TimeLineInnerWidget::updateWheel(QWheelEvent* aEvent)
{
    mEditor->updateWheel(aEvent->angleDelta().y(), mViaPoint.mouseSetting().invertTimeLineScaling);
    updateSize();
}

void TimeLineInnerWidget::updateSize()
{
    // get inner size
    // add enough margin to coordinate height with ObjectTreeWidget.
    QSize size = mEditor->modelSpaceSize() + QSize(0, 128);

    if (mCamera)
    {
        const QSize camsize = mCamera->screenSize();
        if (size.width() < camsize.width()) size.setWidth(camsize.width());
        if (size.height() < camsize.height()) size.setHeight(camsize.height());
    }
    this->resize(size);
    this->update();
}

void TimeLineInnerWidget::updateProjectAttribute()
{
    mEditor->updateProjectAttribute();
    updateTimeCursorPos();
    updateSize();
}

void TimeLineInnerWidget::paintEvent(QPaintEvent* aEvent)
{
    QPainter painter;
    painter.begin(this);
    if (mCamera)
    {
        mEditor->render(painter, *mCamera, aEvent->rect());
    }
    painter.end();
}

void TimeLineInnerWidget::onTimeLineModified(core::TimeLineEvent&, bool)
{
    if (!mOnPasting)
    {
        mCopyTargets = core::TimeLineEvent();
    }
    mEditor->updateKey();
    this->update();
}

void TimeLineInnerWidget::onTreeRestructured(core::ObjectTreeEvent&, bool)
{
    mCopyTargets = core::TimeLineEvent();
}

void TimeLineInnerWidget::onProjectAttributeModified(core::ProjectEvent&, bool)
{
    mCopyTargets = core::TimeLineEvent();
}

void TimeLineInnerWidget::onContextMenuRequested(const QPoint& aPos)
{
    QMenu menu(this);

    mTargets = core::TimeLineEvent();
    if (mEditor->checkContactWithKeyFocus(mTargets, aPos))
    {
        menu.addAction(mCopyKey);
        menu.addSeparator();
        menu.addAction(mDeleteKey);
    }
    else
    {
        mPastePos = aPos;
        mPasteKey->setEnabled(mCopyTargets.hasAnyTargets());
        menu.addAction(mPasteKey);
    }
    menu.exec(this->mapToGlobal(aPos));
    this->update();
}

void TimeLineInnerWidget::onCopyKeyTriggered(bool)
{
    mCopyTargets = mTargets;
}

void TimeLineInnerWidget::onPasteKeyTriggered(bool)
{
    mOnPasting = true;
    if (!mEditor->pasteCopiedKeys(mCopyTargets, mPastePos))
    {
        QMessageBox::warning(nullptr, tr("Operation Error"), tr("Failed to paste keys."));
    }
    mOnPasting = false;
}

void TimeLineInnerWidget::onDeleteKeyTriggered(bool)
{
    mEditor->deleteCheckedKeys(mTargets);
}

} // namespace gui
