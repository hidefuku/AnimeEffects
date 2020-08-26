#include <QMenu>
#include <QMessageBox>
#include "gui/TimeLineEditorWidget.h"
#include "gui/MouseSetting.h"
#include "gui/obj/obj_Item.h"

namespace gui
{
//-------------------------------------------------------------------------------------------------
TimeCursor::TimeCursor(QWidget* aParent)
    : QWidget(aParent)
    , mBodyColor(QColor(230, 230, 230, 180))
    , mEdgeColor(QColor(80, 80, 80, 180))
{
    this->setAutoFillBackground(false);
}

void TimeCursor::setCursorPos(const QPoint& aPos, int aHeight)
{
    const QPoint range(5, 5);
    QRect rect;
    rect.setTopLeft(aPos - range);
    rect.setBottomRight(aPos + range + QPoint(0, aHeight));
    this->setGeometry(rect);
}

void TimeCursor::paintEvent(QPaintEvent* aEvent)
{
    (void)aEvent;
    QPainter painter;
    painter.begin(this);

    const QPoint range(5, 5);
    const QBrush kBrushBody(mBodyColor);
    const QBrush kBrushEdge(mEdgeColor);

    painter.setPen(QPen(kBrushEdge, 1));
    painter.setBrush(kBrushBody);
    painter.drawLine(range + QPoint(0, range.y()), range + QPoint(0, this->geometry().height()));

    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawEllipse(QPointF(range), range.x() - static_cast<qreal>(0.5f), range.y() - static_cast<qreal>(0.5f));

    painter.end();
}

QColor TimeCursor::edgeColor() const
{
    return mEdgeColor;
}

void TimeCursor::setEdgeColor(const QColor &cursorEdgeColor)
{
    mEdgeColor = cursorEdgeColor;
}

QColor TimeCursor::bodyColor() const
{
    return mBodyColor;
}

void TimeCursor::setBodyColor(const QColor &cursorBodyColor)
{
    mBodyColor = cursorBodyColor;
}

//-------------------------------------------------------------------------------------------------
TimeLineEditorWidget::TimeLineEditorWidget(ViaPoint& aViaPoint, QWidget* aParent)
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
    , mTimelineTheme()
{
    mTimeCursor.show();

    mEditor.reset(new ctrl::TimeLineEditor());
    this->setMouseTracking(true);
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    this->connect(
                this, &QWidget::customContextMenuRequested,
                this, &TimeLineEditorWidget::onContextMenuRequested);

    {
        mCopyKey = new QAction(tr("copy key"), this);
        mCopyKey->connect(mCopyKey, &QAction::triggered, this, &TimeLineEditorWidget::onCopyKeyTriggered);

        mPasteKey = new QAction(tr("paste key"), this);
        mPasteKey->connect(mPasteKey, &QAction::triggered, this, &TimeLineEditorWidget::onPasteKeyTriggered);

        mDeleteKey = new QAction(tr("delete key"), this);
        mDeleteKey->connect(mDeleteKey, &QAction::triggered, this, &TimeLineEditorWidget::onDeleteKeyTriggered);
    }
}

void TimeLineEditorWidget::setProject(core::Project* aProject)
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
                    this, &TimeLineEditorWidget::onTimeLineModified);

        mTreeRestructSlot = aProject->onTreeRestructured.connect(
                    this, &TimeLineEditorWidget::onTreeRestructured);

        mProjectAttrSlot = aProject->onProjectAttributeModified.connect(
                    this, &TimeLineEditorWidget::onProjectAttributeModified);
    }
    else
    {
        mProject.reset();
    }

    mEditor->setProject(aProject);
    updateSize();
}

void TimeLineEditorWidget::setFrame(core::Frame aFrame)
{
    mEditor->setFrame(aFrame);

    // particial rendering
    updateTimeCursorPos();
}

core::Frame TimeLineEditorWidget::currentFrame() const
{
    return mEditor->currentFrame();
}

int TimeLineEditorWidget::maxFrame() const
{
    return mEditor->maxFrame();
}

void TimeLineEditorWidget::updateTimeCursorPos()
{
    if (mCamera)
    {
        auto pos = mEditor->currentTimeCursorPos();
        mTimeCursor.setCursorPos(
                    QPoint(pos.x(), pos.y() - static_cast<int>(mCamera->leftTopPos().y())),
                    mCamera->screenHeight());
    }
}

void TimeLineEditorWidget::updateCamera(const core::CameraInfo& aCamera)
{
    mCamera = &aCamera;
    updateSize();
    updateTimeCursorPos();
}

void TimeLineEditorWidget::updateLines(QTreeWidgetItem* aTopNode)
{
    mEditor->clearRows();

    if (!aTopNode || !aTopNode->isExpanded()) return;

    for (int i = 0; i < aTopNode->childCount(); ++i)
    {
        updateLinesRecursive(aTopNode->child(i));
    }
    updateSize();
}

void TimeLineEditorWidget::updateLinesRecursive(QTreeWidgetItem* aItem)
{
    if (!aItem) return;

    const bool isClosedFolder = !aItem->isExpanded() && aItem->childCount() > 0;
    obj::Item* objItem = obj::Item::cast(aItem);
    if (objItem && !objItem->isTopNode())
    {
        int screenTop = mCamera ? static_cast<int>(-mCamera->leftTopPos().y()) : 0;

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

void TimeLineEditorWidget::updateLineSelection(core::ObjectNode* aRepresent)
{
    mEditor->updateRowSelection(aRepresent);
    this->update();
}

bool TimeLineEditorWidget::updateCursor(const core::AbstractCursor& aCursor)
{
    ctrl::TimeLineEditor::UpdateFlags flags = mEditor->updateCursor(aCursor);

    if (flags & ctrl::TimeLineEditor::UpdateFlag_ModView)
    {
        updateTimeCursorPos();
        this->update();
    }
    return (flags & ctrl::TimeLineEditor::UpdateFlag_ModFrame);
}

void TimeLineEditorWidget::updateWheel(QWheelEvent* aEvent)
{
    mEditor->updateWheel(aEvent->angleDelta().y(), mViaPoint.mouseSetting().invertTimeLineScaling);
    updateSize();
}

void TimeLineEditorWidget::updateSize()
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

void TimeLineEditorWidget::updateProjectAttribute()
{
    mEditor->updateProjectAttribute();
    updateTimeCursorPos();
    updateSize();
}

void TimeLineEditorWidget::updateTheme(theme::Theme& aTheme)
{
    Q_UNUSED(aTheme) // TODO
    mTimelineTheme.reset();
}

void TimeLineEditorWidget::paintEvent(QPaintEvent* aEvent)
{
    QPainter painter;
    painter.begin(this);
    if (mCamera)
    {
        mEditor->render(painter, *mCamera, mTimelineTheme, aEvent->rect());
    }
    painter.end();
}

void TimeLineEditorWidget::onTimeLineModified(core::TimeLineEvent&, bool)
{
    if (!mOnPasting)
    {
        mCopyTargets = core::TimeLineEvent();
    }
    mEditor->updateKey();
    this->update();
}

void TimeLineEditorWidget::onTreeRestructured(core::ObjectTreeEvent&, bool)
{
    mCopyTargets = core::TimeLineEvent();
}

void TimeLineEditorWidget::onProjectAttributeModified(core::ProjectEvent&, bool)
{
    mCopyTargets = core::TimeLineEvent();
}

void TimeLineEditorWidget::onContextMenuRequested(const QPoint& aPos)
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

void TimeLineEditorWidget::onCopyKeyTriggered(bool)
{
    mCopyTargets = mTargets;
}

void TimeLineEditorWidget::onPasteKeyTriggered(bool)
{
    mOnPasting = true;
    if (!mEditor->pasteCopiedKeys(mCopyTargets, mPastePos))
    {
        QMessageBox::warning(nullptr, tr("Operation Error"), tr("Failed to paste keys."));
    }
    mOnPasting = false;
}

void TimeLineEditorWidget::onDeleteKeyTriggered(bool)
{
    mEditor->deleteCheckedKeys(mTargets);
}

QColor TimeLineEditorWidget::headerContentColor() const
{
    return mTimelineTheme.headerContentColor();
}

void TimeLineEditorWidget::setHeaderContentColor(const QColor &headerContentColor)
{
    mTimelineTheme.setHeaderContentColor(headerContentColor);
}

QColor TimeLineEditorWidget::headerBackgroundColor() const
{
    return mTimelineTheme.headerBackgroundColor();
}

void TimeLineEditorWidget::setHeaderBackgroundColor(const QColor &headerBackgroundColor)
{
    mTimelineTheme.setHeaderBackgroundColor(headerBackgroundColor);
}

QColor TimeLineEditorWidget::trackColor() const
{
    return mTimelineTheme.trackColor();
}

void TimeLineEditorWidget::setTrackColor(const QColor &trackColor)
{
    mTimelineTheme.setTrackColor(trackColor);
}

QColor TimeLineEditorWidget::trackTextColor() const
{
    return mTimelineTheme.trackTextColor();
}

void TimeLineEditorWidget::setTrackTextColor(const QColor &trackTextColor)
{
    mTimelineTheme.setTrackTextColor(trackTextColor);
}

QColor TimeLineEditorWidget::trackEdgeColor() const
{
    return mTimelineTheme.trackEdgeColor();
}

void TimeLineEditorWidget::setTrackEdgeColor(const QColor &trackEdgeColor)
{
    mTimelineTheme.setTrackEdgeColor(trackEdgeColor);
}

QColor TimeLineEditorWidget::trackSelectColor() const
{
    return mTimelineTheme.trackSelectColor();
}

void TimeLineEditorWidget::setTrackSelectColor(const QColor &trackSelectColor)
{
    mTimelineTheme.setTrackSelectColor(trackSelectColor);
}

QColor TimeLineEditorWidget::trackSeperatorColor() const
{
    return mTimelineTheme.trackSeperatorColor();
}

void TimeLineEditorWidget::setTrackSeperatorColor(const QColor &trackSeperatorColor)
{
    mTimelineTheme.setTrackSeperatorColor(trackSeperatorColor);
}


} // namespace gui
