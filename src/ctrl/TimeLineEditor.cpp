#include <QPainter>
#include "util/TreeIterator.h"
#include "cmnd/ScopedMacro.h"
#include "cmnd/BasicCommands.h"
#include "ctrl/TimeLineEditor.h"
#include "ctrl/CmndName.h"
#include "ctrl/time/time_Renderer.h"

using namespace core;

namespace
{

static const int kTimeLineFpsA = 60;
static const int kTimeLineFpsB = 30;
static const int kTimeLineFpsC = 10;
static const int kTimeLineMargin = 14;
static const int kHeaderHeight = 22;
static const int kDefaultMaxFrame = 600;

}

namespace ctrl
{

//-------------------------------------------------------------------------------------------------
TimeLineEditor::TimeLineEditor()
    : mProject()
    , mRows()
    , mSelectingRow()
    , mTimeMax()
    , mState(State_Standby)
    , mTimeCurrent(kTimeLineMargin)
    , mTimeScale()
    , mFocus(mRows, mTimeScale, kTimeLineMargin)
    , mMoveRef()
    , mMoveFrame()
    , mOnUpdatingKey(false)
    , mShowSelectionRange(false)
{
    mRows.reserve(64);

    const std::array<int, 3> kFrameList =
    {
        kTimeLineFpsA,
        kTimeLineFpsB,
        kTimeLineFpsC
    };
    mTimeScale.setFrameList(kFrameList);

    // reset max frame
    setMaxFrame(kDefaultMaxFrame);
}

void TimeLineEditor::setMaxFrame(int aValue)
{
    mTimeMax = aValue;
    mTimeScale.setMaxFrame(mTimeMax);
    mTimeCurrent.setMaxFrame(mTimeMax);
    mTimeCurrent.setFrame(mTimeScale, core::Frame(0));
}

void TimeLineEditor::setProject(Project* aProject)
{
    clearRows();
    mProject.reset();

    if (aProject)
    {
        mProject = aProject->pointee();
        setMaxFrame(mProject->attribute().maxFrame());
    }
    else
    {
        setMaxFrame(kDefaultMaxFrame);
    }
}

void TimeLineEditor::clearRows()
{
    mRows.clear();
    clearState();
}

void TimeLineEditor::clearState()
{
    mFocus.clear();
    mState = State_Standby;
    mMoveRef = NULL;
    mMoveFrame = 0;
    mShowSelectionRange = false;
}

void TimeLineEditor::pushRow(ObjectNode* aNode, util::Range aWorldTB, bool aClosedFolder)
{
    const int left = kTimeLineMargin;
    const int right = left + mTimeScale.maxPixelWidth();
    const QRect rect(QPoint(left, aWorldTB.min()), QPoint(right, aWorldTB.max()));
    mRows.push_back(TimeLineRow(aNode, rect, aClosedFolder, aNode == mSelectingRow));
}

void TimeLineEditor::updateRowSelection(const core::ObjectNode* aRepresent)
{
    mSelectingRow = aRepresent;
    for (auto& row : mRows)
    {
        row.selecting = (row.node && row.node == aRepresent);
    }
}

void TimeLineEditor::updateKey()
{
    if (!mOnUpdatingKey)
    {
        clearState();
    }
}

void TimeLineEditor::updateProjectAttribute()
{
    clearState();
    if (mProject)
    {
        const int newMaxFrame = mProject->attribute().maxFrame();
        if (mTimeMax != newMaxFrame)
        {
            setMaxFrame(newMaxFrame);

            const int newRowRight = kTimeLineMargin + mTimeScale.maxPixelWidth();
            for (auto& row : mRows)
            {
                row.rect.setRight(newRowRight);
            }
        }
    }
}

TimeLineEditor::UpdateFlags TimeLineEditor::updateCursor(const AbstractCursor& aCursor)
{
    TimeLineEditor::UpdateFlags flags = 0;

    if (!mProject)
    {
        return flags;
    }

    const QPoint worldPoint = aCursor.worldPoint();

    if (aCursor.emitsLeftPressedEvent())
    {
        // a selection range is exists.
        if (mState == State_EncloseKeys)
        {
            if (mFocus.isInRange(worldPoint) && beginMoveKeys(worldPoint))
            {
                mState = State_MoveKeys;
                flags |= UpdateFlag_ModView;
            }
            else
            {
                mShowSelectionRange = false;
                mState = State_Standby;
                flags |= UpdateFlag_ModView;
            }
        }

        // idle state
        if (mState == State_Standby)
        {
            const auto target = mFocus.reset(worldPoint);

            const QVector2D handlePos(mTimeCurrent.handlePos());

            if ((aCursor.screenPos() - handlePos).length() < mTimeCurrent.handleRange())
            {
                mState = State_MoveCurrent;
                flags |= UpdateFlag_ModView;
            }
            else if (aCursor.screenPos().y() < kHeaderHeight)
            {
                mTimeCurrent.setHandlePos(mTimeScale, aCursor.worldPos().toPoint());
                mState = State_MoveCurrent;
                flags |= UpdateFlag_ModView;
                flags |= UpdateFlag_ModFrame;
            }
            else if (target.isValid())
            {
                beginMoveKey(target);
                mState = State_MoveKeys;
                flags |= UpdateFlag_ModView;
            }
            else
            {
                mShowSelectionRange = true;
                mState = State_EncloseKeys;
                flags |= UpdateFlag_ModView;
            }
        }
    }
    else if (aCursor.emitsLeftDraggedEvent())
    {
        if (mState == State_MoveCurrent)
        {
            mTimeCurrent.setHandlePos(mTimeScale, aCursor.worldPos().toPoint());
            flags |= UpdateFlag_ModView;
            flags |= UpdateFlag_ModFrame;
        }
        else if (mState == State_MoveKeys)
        {
            if (!modifyMoveKeys(aCursor.worldPoint()))
            {
                mState = State_Standby;
                mMoveRef = NULL;
                mFocus.clear();
            }
            flags |= UpdateFlag_ModView;
            flags |= UpdateFlag_ModFrame;
        }
        else if (mState == State_EncloseKeys)
        {
            mFocus.update(aCursor.worldPoint());
            flags |= UpdateFlag_ModView;
        }
    }
    else if (aCursor.emitsLeftReleasedEvent())
    {
        if (mState != State_EncloseKeys || !mFocus.hasRange())
        {
            mMoveRef = NULL;
            mState = State_Standby;
            mShowSelectionRange = false;
            flags |= UpdateFlag_ModView;
        }
    }
    else
    {
        if (mState != State_EncloseKeys)
        {
            mFocus.reset(aCursor.worldPoint());
        }
    }

    if (mFocus.viewIsChanged())
    {
        flags |= UpdateFlag_ModView;
    }

    return flags;
}

void TimeLineEditor::beginMoveKey(const time::Focuser::SingleFocus& aTarget)
{
    XC_ASSERT(aTarget.isValid());

    mOnUpdatingKey = true;
    {
        cmnd::ScopedMacro macro(mProject->commandStack(), CmndName::tr("move a key"));

        auto notifier = TimeLineUtil::createMoveNotifier(
                            *mProject, *aTarget.node, aTarget.pos);
        macro.grabListener(notifier);

        mMoveRef = new TimeLineUtil::MoveFrameOfKey(notifier->event());
        mProject->commandStack().push(mMoveRef);
        mMoveFrame = aTarget.pos.index();
    }
    mOnUpdatingKey = false;
}

bool TimeLineEditor::beginMoveKeys(const QPoint& aWorldPos)
{
    bool success = false;
    mOnUpdatingKey = true;
    {
        auto notifier = new TimeLineUtil::Notifier(*mProject);
        notifier->event().setType(TimeLineEvent::Type_MoveKey);

        if (mFocus.select(notifier->event()))
        {
            cmnd::ScopedMacro macro(mProject->commandStack(), CmndName::tr("move keys"));

            macro.grabListener(notifier);
            mMoveRef = new TimeLineUtil::MoveFrameOfKey(notifier->event());
            mProject->commandStack().push(mMoveRef);
            mMoveFrame = mTimeScale.frame(aWorldPos.x() - kTimeLineMargin);
            success = true;
        }
        else
        {
            delete notifier;
            mMoveRef = NULL;
        }
    }
    mOnUpdatingKey = false;
    return success;
}

bool TimeLineEditor::modifyMoveKeys(const QPoint& aWorldPos)
{
    if (mProject->commandStack().isModifiable(mMoveRef))
    {
        const int newFrame = mTimeScale.frame(aWorldPos.x() - kTimeLineMargin);
        const int addFrame = newFrame - mMoveFrame;
        TimeLineEvent modEvent;

        mOnUpdatingKey = true;
        int clampedAdd = addFrame;
        if (mMoveRef->modifyMove(modEvent, addFrame, util::Range(0, mTimeMax), &clampedAdd))
        {
            mMoveFrame = newFrame;
            mFocus.moveBoundingRect(clampedAdd);
            mProject->onTimeLineModified(modEvent, false);
        }
        mOnUpdatingKey = false;
        return true;
    }
    return false;
}

bool TimeLineEditor::checkContactWithKeyFocus(core::TimeLineEvent& aEvent, const QPoint& aPos)
{
    if (mFocus.hasRange() && !mFocus.isInRange(aPos))
    {
        return false;
    }
    return mFocus.select(aEvent);
}

bool TimeLineEditor::pasteCopiedKeys(core::TimeLineEvent& aEvent, const QPoint& aWorldPos)
{
    XC_ASSERT(!aEvent.targets().isEmpty());

    // a minimum frame for key pasting
    auto pasteFrame = mTimeScale.frame(aWorldPos.x() - kTimeLineMargin);

    // a minimum frame in copied keys
    int copiedFrame = mTimeMax;
    for (auto target : aEvent.targets())
    {
        copiedFrame = std::min(copiedFrame, target.pos.index());
    }

    const int frameOffset = pasteFrame - copiedFrame;

    // check validity
    for (auto target : aEvent.targets())
    {
        auto newFrame = target.pos.index() + frameOffset;

        // invalid frame
        if (newFrame < 0 || mTimeMax < newFrame)
        {
            return false;
        }

        // a key already exists.
        auto type = target.pos.type();
        if (target.pos.line()->hasTimeKey(type, newFrame))
        {
            return false;
        }
    }

    mOnUpdatingKey = true;
    {
        cmnd::Stack& stack = mProject->commandStack();

        // create notifier
        auto notifier = new TimeLineUtil::Notifier(*mProject);
        notifier->event() = aEvent;
        notifier->event().setType(TimeLineEvent::Type_CopyKey);

        // push delete keys command
        cmnd::ScopedMacro macro(stack, CmndName::tr("paste keys"));
        macro.grabListener(notifier);

        QMap<const TimeKey*, TimeKey*> parentMap;
        struct ChildInfo { TimeKey* key; TimeKey* parent; };
        QList<ChildInfo> childList;

        for (auto target : aEvent.targets())
        {
            auto type = target.pos.type();
            auto line = target.pos.line();
            XC_PTR_ASSERT(line);

            auto copiedKey = target.pos.key();
            XC_PTR_ASSERT(copiedKey);
            auto parentKey = copiedKey->parent();

            TimeKey* newKey = copiedKey->createClone();

            auto newFrame = copiedKey->frame() + frameOffset;
            newKey->setFrame(newFrame);

            stack.push(new cmnd::GrabNewObject<TimeKey>(newKey));
            stack.push(line->createPusher(type, newFrame, newKey));

            if (newKey->canHoldChild())
            {
                parentMap[copiedKey] = newKey;
            }
            if (parentKey)
            {
                ChildInfo info = { newKey, parentKey };
                childList.push_back(info);
            }
        }
        // connect to parents
        for (auto child : childList)
        {
            auto parent = child.parent;
            // if the parent was also copied, connect to a new parent key.
            auto it = parentMap.find(parent);
            if (it != parentMap.end()) parent = it.value();
            stack.push(new cmnd::PushBackTree<TimeKey>(&parent->children(), child.key));
        }
    }
    mOnUpdatingKey = false;

    clearState();
    return true;
}

void TimeLineEditor::deleteCheckedKeys(core::TimeLineEvent& aEvent)
{
    XC_ASSERT(!aEvent.targets().isEmpty());

    mOnUpdatingKey = true;
    {
        cmnd::Stack& stack = mProject->commandStack();

        // create notifier
        auto notifier = new TimeLineUtil::Notifier(*mProject);
        notifier->event() = aEvent;
        notifier->event().setType(core::TimeLineEvent::Type_RemoveKey);

        // push delete keys command
        cmnd::ScopedMacro macro(stack, CmndName::tr("delete keys"));
        macro.grabListener(notifier);

        for (auto target : aEvent.targets())
        {
            core::TimeLine* line = target.pos.line();
            XC_PTR_ASSERT(line);
            stack.push(line->createRemover(target.pos.type(), target.pos.index(), true));
        }
    }
    mOnUpdatingKey = false;

    clearState();
}

void TimeLineEditor::updateWheel(int aDelta, bool aInvertScaling)
{
    mTimeScale.update(aInvertScaling ? -aDelta : aDelta);
    mTimeCurrent.update(mTimeScale);

    const int lineWidth = mTimeScale.maxPixelWidth();

    for (TimeLineRow& row : mRows)
    {
        row.rect.setWidth(lineWidth);
    }
}

void TimeLineEditor::setFrame(core::Frame aFrame)
{
    mTimeCurrent.setFrame(mTimeScale, aFrame);
}

core::Frame TimeLineEditor::currentFrame() const
{
    return mTimeCurrent.frame();
}

QSize TimeLineEditor::modelSpaceSize() const
{
    int height = kHeaderHeight;

    if (!mRows.empty())
    {
        height += mRows.back().rect.bottom() - mRows.front().rect.top();
    }

    const int width = mTimeScale.maxPixelWidth() + 2 * kTimeLineMargin;

    return QSize(width, height);
}

QPoint TimeLineEditor::currentTimeCursorPos() const
{
    return mTimeCurrent.handlePos();
}

void TimeLineEditor::render(QPainter& aPainter, const CameraInfo& aCamera, const QRect& aCullRect)
{
    if (aCamera.screenWidth() < 2 * kTimeLineMargin) return;

    const QRect camRect(-aCamera.leftTopPos().toPoint(), aCamera.screenSize());
    const QRect cullRect(aCullRect.marginsAdded(QMargins(2, 2, 2, 2))); // use culling

    const int margin = kTimeLineMargin;
    const int bgn = mTimeScale.frame(cullRect.left() - margin - 5);
    const int end = mTimeScale.frame(cullRect.right() - margin + 5);

    time::Renderer renderer(aPainter, aCamera);
    renderer.setMargin(margin);
    renderer.setRange(util::Range(bgn, end));
    renderer.setTimeScale(mTimeScale);

    renderer.renderLines(mRows, camRect, cullRect);
    renderer.renderHeader(kHeaderHeight, kTimeLineFpsA);
    //renderer.renderHandle(mTimeCurrent.handlePos(), mTimeCurrent.handleRange());

    if (mShowSelectionRange)
    {
        renderer.renderSelectionRange(mFocus.visualRect());
    }
}

} // namespace ctrl
